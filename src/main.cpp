#include "ClientOps.h"
#include "NodeServer.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/TCPServer.h"
#include "Poco/Util/ServerApplication.h"
#include "Utils.h"
#include "WebUi.h"
#include "startServer.h"
#include <iomanip>
#include <iostream>

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

class Sdm : public ServerApplication {
  std::vector<std::string> get_files;
  std::vector<std::string> put_files;
  std::unordered_set<std::string> nodes;
  std::unordered_set<std::string> flags;
  Node *self = Node::getLocalhostNode();

  void help(const std::string &name, const std::string &value) {
    std::cerr << "SDM - SLURM Data Manager" << std::endl;

    std::cerr << std::endl;

    std::cerr << "Usage:" << std::endl;

    std::cerr << "  Put file.txt:" << std::endl;
    std::cerr << "    wts --put file.txt" << std::endl;
    std::cerr << "  Get file.txt:" << std::endl;
    std::cerr << "    wts --get file.txt" << std::endl;
    std::cerr << "  Start primary Node Server:" << std::endl;
    std::cerr << "    wts --serve" << std::endl;
    std::cerr << "  Start secondary Node Server:" << std::endl;
    std::cerr << "    wts --serve -node=<primary-ip>" << std::endl;
    std::cerr << std::endl;

    std::cerr << "Options:" << std::endl;

    for (auto op : options()) {
      std::string sparam =
          (op.takesArgument()) ? ("<" + op.argumentName() + ">") : "";
      std::string lparam = (sparam != "") ? ("=" + sparam) : "";
      std::cerr << "  -" << std::setw(10) << std::left
                << (op.shortName() + sparam);
      std::cerr << "--" << std::setw(15) << (op.fullName() + lparam);
      std::cerr << op.description() << std::endl;
    }

    std::cerr << std::endl;
    std::cerr << "(*): Get,Put,Node can be used more than once." << std::endl;

    exit(0);
  }

  void addFile(const std::string &name, const std::string &value) {
    std::cerr << __func__ << std::endl;
    if (name == "get") {
      get_files.push_back(value);
      return;
    }
    if (name == "put") {
      put_files.push_back(value);
      return;
    }
  }

  void addNode(const std::string &name, const std::string &node) {
    nodes.insert(node);
  }

  void setFlag(const std::string &name, const std::string &value) {
    flags.insert(name);
  }

  void addIp(const std::string &name, const std::string &ip) {
    self->addresses.emplace_back(ip);
  }

  void defineOptions(OptionSet &options) {
    options.addOption(Option("cport", "c", "Command Port")
                          .argument("port")
                          .binding("cmd_port"));
    options.addOption(Option("dport", "d", "Data Port")
                          .argument("port")
                          .binding("data_port"));
    options.addOption(Option("wui", "w", "Webui Port")
                          .argument("port", false)
                          .binding("wui_port"));
    options.addOption(
        Option("job_id", "j", "Job id ").argument("id").binding("jid"));

    options.addOption(Option("get", "g", "Get file(*)")
                          .argument("file")
                          .repeatable(true)
                          .callback(OptionCallback<Sdm>(this, &Sdm::addFile)));
    options.addOption(Option("put", "p", "Put file(*)")
                          .argument("file")
                          .repeatable(true)
                          .callback(OptionCallback<Sdm>(this, &Sdm::addFile)));
    options.addOption(Option("serve", "s", "Start Node server")
                          .callback(OptionCallback<Sdm>(this, &Sdm::setFlag)));
    options.addOption(Option("node", "n", "Register to node(*)")
                          .argument("ip")
                          .repeatable(true)
                          .callback(OptionCallback<Sdm>(this, &Sdm::addNode)));
    options.addOption(Option("ip", "i", "Register this node to ip")
                          .argument("ip")
                          .repeatable(true)
                          .callback(OptionCallback<Sdm>(this, &Sdm::addIp)));

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

    uint16_t cmd_port = getPort("cmd_port", 5555);

    uint16_t data_port = getPort("data_port", 2222);

    std::thread *data_server = 0;

    if (flags.count("serve")) {
      TCPServer *tcp = new TCPServer(
          new TCPServerConnectionFactoryImpl<NodeServerHandler>(), cmd_port);
      tcp->start();
      servers["Node "] = tcp;
      logger().information("[Node ] Starting on port %hu", cmd_port);

      data_server =
          StartTheServer(Node::getHostname(), std::to_string(data_port));
    }

    if (wui_port) {
      HTTPServer *http = new HTTPServer(new WebUiFactory, wui_port);
      http->start();
      servers["WebUI"] = http;
      logger().information("[WebUI] Starting on port %hu", wui_port);
    }

    if (nodes.size()) {
      for (auto node : nodes) {
        join_node(self, node, cmd_port);
      }
    }

    if (put_files.size() || get_files.size()) {
      for (auto put : put_files) {
        put_file(self, cmd_port, put);
      }

      for (auto get : get_files) {
        get_file(self, cmd_port, get);
      }
    }

    if (wui_port || flags.count("serve")) {
      logger().information("[main ] Waiting for cntrl-c");
      waitForTerminationRequest();
      logger().information("\n[main ] cntrl-c received");
    }

    for (auto &server : servers) {
      const std::string &name = server.first;
      logger().information("[%s] Stopping", name);
      server.second->stop();
      delete server.second;
      logger().information("[%s] Stopped", name);
    }

    if (data_server) {
      data_server->join();
      delete data_server;
    }

    return Application::EXIT_OK;
  }
};

POCO_SERVER_MAIN(Sdm)
