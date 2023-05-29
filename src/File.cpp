#include "File.h"

File ::File(const std::string &file_name) : file_name(file_name), size(0) {
  nodes.insert(Node::getHostname());
}
void File ::toHTML(std::ostream &os) const {
  os << "<tr><td>" << file_name << "</td>\n";
  os << "<td>" << size << "</td>\n";
  os << "<td>";
  for (auto node : nodes) {
    os << " " << node;
  }
  os << "</td></tr>\n";
}

void File ::updateFrom(const File &update) {
  nodes.insert(update.nodes.begin(), update.nodes.end());
}

bool operator==(const File &lhs, const File &rhs) {
  return lhs.file_name == rhs.file_name;
}

Poco::Net::StreamSocket &operator<<(Poco::Net::StreamSocket &sock,
                                    const File &file) {
  sdm_pack(sock, file.file_name);
  sdm_pack_size(sock, file.nodes.size());
  for (auto &node : file.nodes)
    sdm_pack(sock, node);
  return sock;
}
Poco::Net::StreamSocket &operator>>(Poco::Net::StreamSocket &sock, File &file) {
  file.file_name = sdm_unpack(sock);
  size_t nodes = sdm_unpack_size(sock);
  file.nodes.clear();
  file.nodes.reserve(nodes);

  while (nodes--) {
    file.nodes.insert(sdm_unpack(sock));
  }
  return sock;
}

File ::FileState File ::files;
