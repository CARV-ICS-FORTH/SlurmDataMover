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
