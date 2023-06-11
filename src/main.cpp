#include "Poco/Environment.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/TCPServer.h"
#include "Poco/Util/ServerApplication.h"
#include "Redis.h"
#include "Utils.h"
#include "WebUi.h"
#include "startServer.h"
#include <iomanip>
#include <iostream>
#include <regex>

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

class Sdm : public ServerApplication {
  std::vector<std::string> get_files;
  std::vector<std::string> put_files;
  std::unordered_set<std::string> flags;
  Node &localhost = Node::getLocalhostNode();
  std::string redis_node = Node::getHostname();

  void help(const std::string &name = "", const std::string &value = "") {
    std::cerr << "SDM - SLURM Data Manager" << std::endl;

    std::cerr << std::endl;

    std::cerr << "Usage:" << std::endl;

    std::cerr << "  Put file.txt:" << std::endl;
    std::cerr << "    wts --put file.txt" << std::endl;
    std::cerr << "  Get file.txt:" << std::endl;
    std::cerr << "    wts --get file.txt" << std::endl;
    std::cerr << "  Start Node Server:" << std::endl;
    std::cerr << "    wts --serve" << std::endl;
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
    std::cerr << "You must pass atleast one of Get,Put or Serve.";
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
    static std::regex fmt("[^@]+@[^@]+");

    if (!std::regex_match(mount, fmt)) {
      std::cerr << "Improper mount format for \'" << mount << "\'" << std::endl;
      help();
    }

    std::string mname = mount.substr(0, mount.find_first_of('@'));
    std::string mpath = mount.substr(name.size());
    localhost.mounts[mname] = mpath;
    Log::Info("Node", "Added mount '%s' at '%s'", mname, mpath);
  }

  void setFlag(const std::string &name, const std::string &value) {
    flags.insert(name);
  }

  void addIp(const std::string &name, const std::string &ip) {
    localhost.addresses.emplace_back(ip);
  }

  void defineOptions(OptionSet &options) {
    options.addOption(Option("redis", "r", "Redis Port")
                          .argument("port")
                          .binding("redis_port"));
    options.addOption(Option("dport", "d", "Data Port")
                          .argument("port")
                          .binding("data_port"));
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
    uint16_t wui_port = getPort("wui_port", 0);
    std::map<std::string, TCPServer *> servers;

    uint16_t redis_port = getPort("redis_port", 6379);

    uint16_t data_port = getPort("data_port", 2222);

    std::thread *data_server = 0;

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
      data_server =
          StartTheServer(Node::getHostname(), std::to_string(data_port));

      if (Environment::has("SLURM_JOB_NODELIST")) {
        std::string node_env = Environment::get("SLURM_JOB_NODELIST");
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

    if (wui_port) {
      HTTPServer *http = new HTTPServer(new WebUiFactory, wui_port);
      http->start();
      servers["WebUI"] = http;
      Log::Info("WebUI", "Starting on port %hu", wui_port);
    }

    if (put_files.size() || get_files.size()) {
      for (auto put : put_files) {
        File file = Redis::getFile(put);
        if (file == File::NotFound) {
          file = File(put);
          Redis::add(file);
        }
      }

      for (auto get : get_files) {
        File file = Redis::getFile(get);
        if (file == File::NotFound) {
          Log::Error("Get file: %s not found", get);
        } else {
          Log::Info("Get file: %s found", get);
        }
      }
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
      server.second->stop();
      delete server.second;
      Log::Info(name, "Stopped");
    }

    if (data_server) {
      data_server->join();
      delete data_server;
    }

    return Application::EXIT_OK;
  }

public:
  Sdm() : ServerApplication() { Log::setLoggger(logger()); }
};

POCO_SERVER_MAIN(Sdm)
