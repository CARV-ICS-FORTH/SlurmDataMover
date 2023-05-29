#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Poco/Net/IPAddress.h>

struct Node {
  Node(const std::string name);
  typedef std::unordered_set<Node> Nodes;
  typedef std::set<Node> SortedNodes;
  std::string name;
  std::vector<Poco::Net::IPAddress> addresses;

  Poco::Net::IPAddress getIpAddress() const;

  void toHTML(std::ostream &os) const;

  static std::string getHostname();
  static Node *getLocalhostNode();
  static const Node *get(std::string hostname);
  static bool addNode(const Node &node);
  static const Nodes &getNodes();
  static SortedNodes getNodesSorted();

  static const Node *getFromIp(std::string ip);

private:
  void resolve();
  static std::unordered_map<std::string, std::string> ip2hostname;
  static Nodes nodes;
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
