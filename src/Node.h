#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "JSONable.h"
#include <Poco/Net/IPAddress.h>

struct Node : public JSONable {
  Node(const std::string name);
  typedef std::unordered_set<Node> Nodes;
  typedef std::set<Node> SortedNodes;
  std::string name;
  std::vector<Poco::Net::IPAddress> addresses;
  std::unordered_map<std::string, std::string> mounts;

  Poco::Net::IPAddress getIpAddress() const;

  void toHTML(std::ostream &os) const;

  static std::string getHostname();
  static Node &getLocalhostNode();
  static const Node *get(std::string hostname);

  bool parseMountPath(const std::string &mount_path, std::string &file,
                      std::string &mount);

  bool parseMountPath(const std::string &mount_path, std::string &file);

  std::string normalizeMountPath(std::string mount_path);

  operator bool() const;

  IMPLEMENTS_JSONable;

  static const Node NotFound;

private:
  void resolve();
};

namespace std {
template <> struct hash<Node> {
  size_t operator()(const Node node) const {
    return std::hash<std::string>()(node.name);
  }
};
} // namespace std

bool operator==(const Node &lhs, const Node &rhs);
bool operator!=(const Node &lhs, const Node &rhs);
bool operator<(const Node &lhs, const Node &rhs);
