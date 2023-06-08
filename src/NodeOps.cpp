#include "NodeOps.h"
#include "ClientOps.h"
#include "Poco/Util/Application.h"
#include "Redis.h"
#include "Utils.h"
#include "getData.h"

using namespace Poco::Net;
using namespace Poco::Util;

void file_update(const File &file, Poco::Net::IPAddress remote,
                 uint16_t cmd_port) {
  Application &app = Application::instance();
  StreamSocket ss;
  app.logger().information("[Update] Connect(%s,%d)", remote.toString(),
                           (int)cmd_port);
  ss.connect(SocketAddress(remote, cmd_port));

  sdm_pack(ss, "update");

  ss << file;
}

void broadcast_file(const File &file, std::string &aka, Poco::Logger &log) {
  Node &self = Node::getLocalhostNode();

  log.information("[Node ] Brodcast file %s from %s%s", file.file_name,
                  self.addresses[0].toString(), aka);

  for (auto &node : Redis::getAllNodes()) {
    if (node != self)
      file_update(file, node.addresses[0], 5555);
  }
}

void handle_put(StreamSocket &sock, std::string &aka, Poco::Logger &log) {
  std::string file;
  file = sdm_unpack(sock);

  bool exists = File::files.count(file);
  log.information("[Node ] Put %s file %s from %s%s",
                  std::string((exists) ? "existing" : "new"), file,
                  sock.peerAddress().toString(), aka);

  if (exists) {
    ((File &)*File::files.find(file)).nodes.insert(Node::getHostname());
    sdm_pack(sock, "Exists");
  } else {
    File::files.emplace(file);
    sdm_pack(sock, "Create");
  }

  broadcast_file(*(File::files.find(file)), aka, log);
}

void handle_get(StreamSocket &sock, std::string &aka, Poco::Logger &log) {
  std::string file_name;
  file_name = sdm_unpack(sock);

  log.information("[Node ] Get file %s from %s%s - Start", file_name,
                  sock.peerAddress().toString(), aka);

  bool found = File::files.count(file_name) == 1;
  std::string found_str = (found) ? "true" : "false";
  std::string host = "NO_HOST";
  std::string path = "NO_PATH";

  if (found) {
    const File &file = *File::files.find(file_name);
    host = *file.nodes.begin();
    path = "??";
  }

  log.information("[Node ] Get file %s from %s%s - %s on %s at %s", file_name,
                  sock.peerAddress().toString(), aka,
                  std::string((found) ? "Found" : "Missing"), host, path);

  if (found) {
    std::vector<std::string> files = {file_name};
    getDataFile(host, 2222, files);
  }

  sdm_pack(sock, found_str);
  sdm_pack(sock, host);
  sdm_pack(sock, path);
}

void handle_join(StreamSocket &sock, std::string &aka, Poco::Logger &log) {

  std::string hostname = sdm_unpack(sock); // Send Hostname

  Node new_node(hostname);

  std::string addr_cnt_str = sdm_unpack(sock); // Number of addresses

  int addr_cnt = std::atoi(addr_cnt_str.c_str());

  new_node.addresses.reserve(addr_cnt);

  for (; addr_cnt; addr_cnt--) {
    std::string addr = sdm_unpack(sock);
    log.information("[Node ] Join from %s of node %s - Addr: %s",
                    sock.peerAddress().toString(), aka, addr);

    new_node.addresses.emplace_back(addr);
  }

  if (!Redis::addNode(new_node))
    log.information("[Node ] Join from %s of existing node %s",
                    sock.peerAddress().toString(), aka);
  else {
    log.information("[Node ] Join from %s of new node (%s)",
                    sock.peerAddress().toString(), new_node.name);

    const Node &self = Node::getLocalhostNode();

    for (auto &node : Redis::getAllNodes()) {
      if (node != self)
        join_node(self, new_node.getIpAddress().toString(), 5555);
    }
  }
}

void handle_file_update(StreamSocket &sock, std::string &aka,
                        Poco::Logger &log) {
  File update("temp");

  bool exists = File::files.count(update.file_name) == 1;

  log.information("[Update] Update from %s of node %s",
                  sock.peerAddress().toString(), aka);

  sock >> update;

  if (!exists)
    File::files.insert(update);
  else {
    ((File &)*File::files.find(update)).updateFrom(update);
  }
}

std::unordered_map<std::string, node_op *> node_op_map = {
    {"get", handle_get},
    {"put", handle_put},
    {"join", handle_join},
    {"update", handle_file_update}};
