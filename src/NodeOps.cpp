#include "NodeOps.h"
#include "ClientOps.h"
#include "Redis.h"
#include "Utils.h"
#include "getData.h"

using namespace Poco::Net;

void file_update(const File &file, Poco::Net::IPAddress remote,
                 uint16_t cmd_port) {
  StreamSocket ss;
  Log::Info("Update", "Connect(%s,%d)", remote.toString(), (int)cmd_port);
  ss.connect(SocketAddress(remote, cmd_port));

  sdm_pack(ss, "update");

  ss << file;
}

void broadcast_file(const File &file, std::string &aka) {
  Node &self = Node::getLocalhostNode();

  Log::Info("Node", "Brodcast file %s from %s%s", file.file_name,
            self.addresses[0].toString(), aka);

  for (auto &node : Redis::getAllNodes()) {
    if (node != self)
      file_update(file, node.addresses[0], 5555);
  }
}

void handle_put(StreamSocket &sock, std::string &aka) {
  std::string file;
  file = sdm_unpack(sock);

  bool exists = File::files.count(file);
  Log::Info("Node", "Put %s file %s from %s%s",
            std::string((exists) ? "existing" : "new"), file,
            sock.peerAddress().toString(), aka);

  if (exists) {
    ((File &)*File::files.find(file)).nodes.insert(Node::getHostname());
    sdm_pack(sock, "Exists");
  } else {
    File::files.emplace(file);
    sdm_pack(sock, "Create");
  }

  broadcast_file(*(File::files.find(file)), aka);
}

void handle_get(StreamSocket &sock, std::string &aka) {
  std::string file_name;
  file_name = sdm_unpack(sock);

  Log::Info("Node", "Get file %s from %s%s - Start", file_name,
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

  Log::Info("Node", "Get file %s from %s%s - %s on %s at %s", file_name,
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

void handle_file_update(StreamSocket &sock, std::string &aka) {
  File update("temp");

  bool exists = File::files.count(update.file_name) == 1;

  Log::Info("Update", "Update from %s of node %s",
            sock.peerAddress().toString(), aka);

  sock >> update;

  if (!exists)
    File::files.insert(update);
  else {
    ((File &)*File::files.find(update)).updateFrom(update);
  }
}

std::unordered_map<std::string, node_op *> node_op_map = {
    {"get", handle_get}, {"put", handle_put}, {"update", handle_file_update}};
