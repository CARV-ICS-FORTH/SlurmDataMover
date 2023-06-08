#include "ClientOps.h"
#include "Utils.h"

using namespace Poco::Net;

void put_file(const Node &local_server, uint16_t cmd_port, std::string file) {
  StreamSocket ss;
  ss.connect(SocketAddress(local_server.getIpAddress(), cmd_port));

  Log::Info("PUT", "Start - %s", file);
  sdm_pack(ss, "put");
  sdm_pack(ss, file);
  std::string status = sdm_unpack(ss);
  Log::Info("PUT", "%s - %s", file, status);
}

void get_file(const Node &local_server, uint16_t cmd_port, std::string file) {
  StreamSocket ss;
  Log::Info("GET", "Connect(%s,%d)", local_server.getIpAddress().toString(),
            (int)cmd_port);
  ss.connect(SocketAddress(local_server.getIpAddress(), cmd_port));

  sdm_pack(ss, "get");
  sdm_pack(ss, file);

  std::string found = sdm_unpack(ss);
  std::string host = sdm_unpack(ss);
  std::string path = sdm_unpack(ss);

  if (found == "true")
    Log::Info("GET", "Found %s at %s in %s", file, host, path);
  else
    Log::Info("GET", "Missn %s at %s in %s", file, host, path);
}

void join_node(const Node &local_server, std::string remote,
               uint16_t cmd_port) {
  Log::Info("Node", "Join %s at node %s", local_server.name, remote);
  StreamSocket ss;

  ss.connect(SocketAddress(remote, cmd_port));

  sdm_pack(ss, "join");

  sdm_pack(ss, local_server.name); // Send Hostname

  sdm_pack(
      ss, std::to_string(local_server.addresses.size())); // Number of addresses

  for (auto &ip : local_server.addresses) // Send all addresses
    sdm_pack(ss, ip.toString());
}
