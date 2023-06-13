#include "BulkServer.h"
#include "Poco/Environment.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Process.h"
#include "Poco/StringTokenizer.h"
#include "Poco/TemporaryFile.h"
#include "Poco/Util/ServerApplication.h"
#include "Redis.h"
#include "Utils.h"
#include "WebUi.h"
#include <iomanip>
#include <iostream>
using namespace Poco::Net;
using namespace Poco::Util;

void fetch_files(std::vector<std::string> files, uint16_t port) {
  Poco::TemporaryFile temp_folder;
  temp_folder.keep();
  temp_folder.createDirectory();
  Log::Info("Env", "Work folder %s", temp_folder.path());
  for (auto file : files) {
    Poco::File target(Node::getLocalhostNode().normalizeMountPath(file));
    std::string local_file_name;
    Node::getLocalhostNode().parseMountPath(file, local_file_name);
    Poco::File local_file(local_file_name);
    Poco::File link(temp_folder.path() + "/" + local_file_name);
    Log::Info("Env", "Link for %s at %s -> %s", file, link.path(),
              target.path());
    if (!link.exists())
      target.linkTo(link.path());
    else
      Log::Info("Env", "Link for %s at %s exists", file, link.path());

    if (target.exists())
      Log::Info("Env", "Target for %s at %s already exists", file,
                target.path());
    else {
      Log::Info("Env", "Target for %s at %s must be fetched", file,
                target.path());
      if (local_file.exists()) {
        Log::Info("Env", "Target for %s at %s copied from nfs %s", file,
                  target.path(), local_file.path());
        local_file.copyTo(target.path());
      } else {
        Log::Info(
            "Env",
            "Target for %s at %s not found localy, requesting file from others",
            file, target.path());
        Redis::requestFiles(files, port);
      }
    }
  }
}

class Sdm : public ServerApplication {
  std::vector<std::string> get_files;
  std::vector<std::string> put_files;
  std::unordered_set<std::string> flags;
  Node &localhost = Node::getLocalhostNode();
  std::string redis_node = Node::getHostname();
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
    std::cerr << "You must pass at least one of Get,Put or Serve.";
    std::cerr << std::endl;

    exit(0);
  }

  void addFile(const std::string &name, const std::string &value) {
    flags.insert(name);
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
    Log::Info("Node", "Added mount '%s' at '%s'", mname, mpath);
  }

  void setFlag(const std::string &name, const std::string &value) {
    flags.insert(name);
  }

  void addIp(const std::string &name, const std::string &ip) {
    localhost.addresses.emplace_back(ip);
  }

  void addExec(const std::string &name, const std::string &cmd) {
    Log::Info("Node", "Added exec command '%s'", cmd);
    exec = cmd;
  }

  void defineOptions(OptionSet &options) {
    options.addOption(Option("redis", "r", "Redis Port")
                          .argument("port")
                          .binding("redis_port"));
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

    options.addOption(Option("exec", "e", "Command to execute")
                          .argument("command")
                          .repeatable(true)
                          .callback(OptionCallback<Sdm>(this, &Sdm::addExec)));

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

  int main(const std::vector<std::string> &) {
    uint16_t wui_port = getPort("wui_port");
    std::map<std::string, TCPServer *> servers;

    uint16_t redis_port = getPort("redis_port", 6379);

    uint16_t bulk_port = getPort("bulk_port");

    if (flags.size() == 0) {
      help();
      return Application::EXIT_USAGE;
    }

    Redis::connect(redis_node, redis_port);

    bool ping = Redis::pingRedis();

    Log::Info("Main", "Redis ping %s ",
              std::string(ping ? "Succesful" : "Failed"));

    Redis::add(localhost);

    if (flags.count("serve")) {
      servers["Bulk"] = new BulkServer(bulk_port);
      bulk_port = servers["Bulk"]->port();

      if (Poco::Environment::has("SLURM_JOB_NODELIST")) {
        std::string node_env = Poco::Environment::get("SLURM_JOB_NODELIST");
        node_env = node_env.substr(0, node_env.find_first_of(","));
        std::string head;
        bool after_cage = false;
        for (auto c : node_env)
          switch (c) {
          case '[':
            after_cage = true;
            break;
          case '-':
            if (after_cage)
              goto done;
          default:
            head += c;
          }

      done:
        Log::Info("Node", "Node from enviroment %s", head);
      }
    } else {
      if (!put_files.size() && !get_files.size()) {
        help("", "");
        return Application::EXIT_USAGE;
      }
    }

    if (wui_port)
      servers["WebUI"] = new HTTPServer(new WebUiFactory, wui_port);

    for (auto server : servers) {
      server.second->start();
      Log::Info(server.first, "Starting on port %hu", server.second->port());
    }

    if (put_files.size() || get_files.size()) {
      for (auto put : put_files) {
        File file = Redis::getFile(put);
        if (file == File::NotFound) {
          Log::Info("PUT", "New File: %s", put);
        } else {
          Log::Info("PUT", "Old File: %s", put);
        }
        file = File(put);
        Redis::add(file);
      }

      fetch_files(get_files, bulk_port);
    }

    std::vector<std::string> args;

    Poco::StringTokenizer stk(exec, " ");

    for (auto tok : stk)
      args.push_back(tok);

    std::cerr << "Launch:" << Poco::Process::launch(*stk.begin(), args).wait()
              << std::endl;

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
