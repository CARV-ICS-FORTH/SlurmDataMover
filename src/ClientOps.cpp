#include "ClientOps.h"

#include "Poco/Util/Application.h"
#include "Utils.h"

using namespace Poco::Util;
using namespace Poco::Net;

void put_file(const Node *local_server, uint16_t cmd_port, std::string file) {
  Application &app = Application::instance();
  StreamSocket ss;
  ss.connect(SocketAddress(local_server->getIpAddress(), cmd_port));

  app.logger().information("[PUT  ] Start - %s", file);
  sdm_pack(ss, "put");
  sdm_pack(ss, file);
  std::string status = sdm_unpack(ss);
  app.logger().information("[PUT  ] %s - %s", file, status);
}

void get_file(const Node *local_server, uint16_t cmd_port, std::string file) {
  Application &app = Application::instance();
  StreamSocket ss;
  app.logger().information("[GET] Connect(%s,%d)",
                           local_server->getIpAddress().toString(),
                           (int)cmd_port);
  ss.connect(SocketAddress(local_server->getIpAddress(), cmd_port));

  sdm_pack(ss, "get");
  sdm_pack(ss, file);

  std::string found = sdm_unpack(ss);
  std::string host = sdm_unpack(ss);
  std::string path = sdm_unpack(ss);

  if (found == "true")
    app.logger().information("[GET  ] Found %s at %s in %s", file, host, path);
  else
    app.logger().information("[GET  ] Missn %s at %s in %s", file, host, path);
}

void join_node(const Node *local_server, std::string remote,
               uint16_t cmd_port) {
  Application &app = Application::instance();
  Poco::Logger &log = app.logger();
  log.information("[Node ] Join %s at node %s", local_server->name, remote);
  StreamSocket ss;

  ss.connect(SocketAddress(remote, cmd_port));

  sdm_pack(ss, "join");

  sdm_pack(ss, local_server->name); // Send Hostname

  sdm_pack(ss, std::to_string(
                   local_server->addresses.size())); // Number of addresses

  for (auto &ip : local_server->addresses) // Send all addresses
    sdm_pack(ss, ip.toString());
}
