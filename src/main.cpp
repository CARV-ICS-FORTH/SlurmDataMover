#include "Bulk.h"
#include "Log.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Path.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/TemporaryFile.h"
#include "Poco/Util/ServerApplication.h"
#include "Redis.h"
#include "Utils.h"
#include "WebUi.h"
#include <fstream>
#include <iomanip>
#include <iostream>
using namespace Poco::Net;
using namespace Poco::Util;

std::string build_workdir(std::vector<std::string> files, bool keep = true) {
  Poco::TemporaryFile temp_folder;
  if (keep)
    temp_folder.keepUntilExit();
  temp_folder.createDirectory();
  Log::Info("Env", "Work folder %s", temp_folder.path());
  for (auto file : files) {
    Poco::File target(Node::getLocalhostNode().normalizeMountPath(file));
    std::string local_file_name;
    Node::getLocalhostNode().parseMountPath(file, local_file_name);
    Poco::File local_file(local_file_name);
    Poco::File link(Poco::Path(temp_folder.path()).append(local_file_name));
    Log::Info("Env", "Link for %s at %s -> %s", file, link.path(),
              target.path());
    if (!link.exists())
      target.linkTo(link.path());
    else
      Log::Info("Env", "Link for %s at %s exists", file, link.path());
  }

  return temp_folder.path();
}

void drop_caches() {
  executeProgram("sync", "/");
  std::ofstream ofs("/proc/sys/vm/drop_caches");
  ofs << "3";
  ofs.close();
}

void fetch_files(std::vector<std::string> files) {
  BulkReciever::FileList req_files;
  std::vector<std::string> file_ids;
  for (auto file : files) {
    Poco::File target(Node::getLocalhostNode().normalizeMountPath(file));
    std::string local_file_name;
    Node::getLocalhostNode().parseMountPath(file, local_file_name);
    Poco::File local_file(local_file_name);

    if (target.exists())
      Log::Info("Env", "Target for %s at %s already exists", file,
                target.path());
    else {
      Log::Info("Env", "Target for %s at %s must be fetched", file,
                target.path());
      if (local_file.exists()) {
        Timeit _t("Local copy " + std::string(file) + " at " + target.path());
        Log::Info("Env", "Target for %s at %s copied from %s", file,
                  target.path(),
                  Poco::Path(local_file.path()).absolute().toString());
        local_file.copyTo(target.path());
      } else {
        Log::Info(
            "Env",
            "Target for %s at %s not found localy, requesting file from others",
            file, target.path());
        req_files[local_file_name] = target.path();
        file_ids.push_back(local_file_name);
      }
    }
  }

  if (file_ids.size()) {
    BulkReciever br(req_files);
    Redis::requestFiles(file_ids, br.port());
    br.wait();
  }
}

class Sdm : public ServerApplication {
  std::vector<std::string> all_files;
  std::vector<std::string> get_files;
  std::vector<std::string> put_files;
  std::unordered_set<std::string> flags;
  Node &localhost = Node::getLocalhostNode();
  SocketAddress redis_sa = SocketAddress(Node::getHostname(), 6379);
  std::string exec;

  void help(const std::string &name = "", const std::string &value = "") {
    std::cerr << "SDM - SLURM Data Manager" << std::endl;

    std::cerr << std::endl;

    std::cerr << "Usage:" << std::endl;

    std::cerr << "  Put file.txt:" << std::endl;
    std::cerr << "    sdm --put file.txt" << std::endl;
    std::cerr << "  Get file.txt:" << std::endl;
    std::cerr << "    sdm --get file.txt" << std::endl;
    std::cerr << "  Start Node Server:" << std::endl;
    std::cerr << "    sdm --serve" << std::endl;
    std::cerr << std::endl;

    std::cerr << "Options:" << std::endl;

    for (auto op : options()) {
      std::string sparam =
          (op.takesArgument()) ? ("<" + op.argumentName() + ">") : "";
      std::string lparam = (sparam != "") ? ("=" + sparam) : "";
      std::cerr << "  -" << std::setw(15) << std::left
                << (op.shortName() + sparam);
      std::cerr << "--" << std::setw(20) << (op.fullName() + lparam);
      std::cerr << op.description() << std::endl;
    }

    std::cerr << std::endl;
    std::cerr << "You must pass at least one of Get,Put,Serve or Wui.";
    std::cerr << std::endl;

    exit(0);
  }

  void addFile(const std::string &name, const std::string &value) {
    flags.insert(name);

    all_files.push_back(value);

    if (name == "get") {
      get_files.push_back(value);
      return;
    }
    if (name == "put") {
      put_files.push_back(value);
      return;
    }
  }

  void addMount(const std::string &name, const std::string &mount) {
    std::string mname;
    std::string mpath;

    if (!localhost.parseMountPath(mount, mname, mpath))
      help();

    localhost.mounts[mname] = mpath;
  }

  void setFlag(const std::string &name, const std::string &value) {
    flags.insert(name);
  }

  void addIp(const std::string &name, const std::string &ip) {
    localhost.addresses.emplace(ip);
  }

  void addExec(const std::string &name, const std::string &cmd) {
    flags.insert(name);
    exec = cmd;
  }

  void defineOptions(OptionSet &options) {

    options.addOption(Option("redis", "r", "Redis socket address")
                          .argument("host:port")
                          .binding("redis"));
    options.addOption(
        Option("bulk", "b", "Bulk Port").argument("port").binding("bulk_port"));

    options.addOption(Option("wui", "w", "Webui Port")
                          .argument("port", false)
                          .binding("wui_port"));
    options.addOption(Option("get", "g", "Get file")
                          .argument("file")
                          .repeatable(true)
                          .callback(OptionCallback<Sdm>(this, &Sdm::addFile)));
    options.addOption(Option("put", "p", "Put file")
                          .argument("file")
                          .repeatable(true)
                          .callback(OptionCallback<Sdm>(this, &Sdm::addFile)));
    options.addOption(Option("serve", "s", "Start Node server")
                          .callback(OptionCallback<Sdm>(this, &Sdm::setFlag)));
    options.addOption(Option("ip", "i", "Extra IP for this node")
                          .argument("ip")
                          .repeatable(true)
                          .callback(OptionCallback<Sdm>(this, &Sdm::addIp)));

    options.addOption(Option("mount", "m", "Add mount to this node")
                          .argument("name@path")
                          .repeatable(true)
                          .callback(OptionCallback<Sdm>(this, &Sdm::addMount)));

    options.addOption(Option("nocache", "n", "Drop FS caches before exec")
                          .callback(OptionCallback<Sdm>(this, &Sdm::setFlag)));

    options.addOption(Option("exec", "e", "Command to execute")
                          .argument("command")
                          .repeatable(true)
                          .callback(OptionCallback<Sdm>(this, &Sdm::addExec)));

    ServerApplication::defineOptions(options);

    options.addOption(Option("help", "h", "Print Help")
                          .callback(OptionCallback<Sdm>(this, &Sdm::help)));
  }

  void initialize(Application &self) {
    loadConfiguration();
    ServerApplication::initialize(self);
  }

  uint16_t getPort(const std::string &port, uint16_t def = 0) {
    return static_cast<uint16_t>(config().getUInt(port, def));
  }

  bool initSocketAddress(SocketAddress &sa, std::string bind) {
    if (config().has(bind) == false)
      return true;
    try { // port only format - use default host
      uint16_t port = config().getUInt(bind);
      SocketAddress temp(sa.host(), port);
      sa = temp;
      return true;
    } catch (Poco::Exception &e) {
    }
    try { // host and port format
      std::string hostAndPort = config().getString(bind);
      SocketAddress temp(hostAndPort);
      sa = temp;
      return true;
    } catch (Poco::Exception &e) {
    }
    try { // host only format - use default port
      std::string host = config().getString(bind);
      SocketAddress temp(host, sa.port());
      sa = temp;
      return true;
    } catch (Poco::Exception &e) {
    }
    std::cerr << "Invalid " << bind << " socket address '"
              << config().getString(bind) << "'" << std::endl;
    return false;
  }

  int main(const std::vector<std::string> &) {
    int return_code = Application::EXIT_OK;
    uint16_t wui_port = getPort("wui_port");
    std::map<std::string, TCPServer *> servers;

    if (!initSocketAddress(redis_sa, "redis")) {
      help();
      return Application::EXIT_USAGE;
    }

    Redis::connect(redis_sa.host().toString(), redis_sa.port());

    uint16_t bulk_port = getPort("bulk_port");

    if (flags.size() == 0 && !wui_port) {
      help();
      return Application::EXIT_USAGE;
    }

    if (!isInteractive()) {
      logger().setChannel(new Poco::SimpleFileChannel("/tmp/sdm.log"));
      Log::Info("Init", "Running as a daemon");
    }

    bool ping = Redis::pingRedis();

    Log::Info("Main", "Redis ping %s ",
              std::string(ping ? "Succesful" : "Failed"));

    if (flags.count("serve")) {
      Redis::add(localhost);

      for (auto mount : localhost.mounts) {
        Poco::File mount_dir(mount.second);
        bool exists = mount_dir.exists();

        Log::Info("Node", "%s mount '%s' at '%s'",
                  std::string(exists ? "Added existing" : "Created"),
                  mount.first, mount.second);

        if (!mount_dir.exists()) {
          Log::Info("Node", "Mount %s at %s did not exist, creating",
                    mount.first, mount.second);
          mount_dir.createDirectories();
        }
      }

      servers["Bulk"] = new BulkSender(bulk_port);
      bulk_port = servers["Bulk"]->port();
    } else {
      if (!put_files.size() && !get_files.size() && !wui_port) {
        help();
        return Application::EXIT_USAGE;
      }
    }

    if (!Redis::get(localhost)) {
      Log::Error("Node",
                 "Could not get node info for %s (is local sdm server running)",
                 localhost);
    }

    if (wui_port)
      servers["Http"] = new HTTPServer(new WebUiFactory, wui_port);

    for (auto server : servers) {
      server.second->start();
      Log::Info(server.first, "Starting on port %hu", server.second->port());
    }

    if (put_files.size() || get_files.size()) {
      fetch_files(get_files);
    }

    if (flags.count("nocache")) {
      drop_caches();
    }

    if (flags.count("exec")) {
      std::string cwd = build_workdir(all_files);

      return_code = executeProgram(exec, cwd);
    }

    if (wui_port || flags.count("serve")) {
      Log::Info("Main", "Waiting for cntrl-c");
      waitForTerminationRequest();
      std::cerr << std::endl;
      Log::Info("Main", "cntrl-c received");
    }

    for (auto &server : servers) {
      const std::string &name = server.first;
      Log::Info(name, "Stopping");
      delete server.second;
      Log::Info(name, "Stopped");
    }

    return Application::EXIT_OK;
  }

public:
  Sdm() : ServerApplication() { Log::setLoggger(logger()); }
};

POCO_SERVER_MAIN(Sdm)
