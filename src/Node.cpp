#include "Node.h"

#include "Poco/Net/DNS.h"
#include "Utils.h"
#include <iostream>

using Poco::Net::DNS;
using Poco::Net::HostEntry;

Node ::Node(const std::string name) : name(name), addresses() {}

void Node ::resolve() {
  return;
  const HostEntry &entry = DNS::thisHost();

  const auto &addrs = entry.addresses();

  addresses.reserve(addrs.size());

  for (auto &addr : addrs) {
    addresses.emplace_back(addr);
    ip2hostname[addr.toString()] = name;
  }
}

std::string Node ::getHostname() {
  static std::string hostname = "";

  if (hostname == "") {
    const HostEntry &self = DNS::thisHost();
    hostname = self.name();
  }
  return hostname;
}

Node *Node ::getLocalhostNode() {
  std::string self = getHostname();
  if (nodes.count(self) == 0) {
    nodes.insert(self);
    auto ret = &(*nodes.find(self));
    ((Node *)ret)->resolve();
    return (Node *)ret;
  }
  return (Node *)&(*nodes.find(self));
}

Poco::Net::IPAddress Node ::getIpAddress() const { return addresses.at(0); }

const Node *Node ::getFromIp(std::string ip) {
  if (ip2hostname.count(ip))
    return &(*nodes.find(ip2hostname[ip]));

  if (ip == "127.0.0.1")
    return getLocalhostNode();

  return 0;
}

void Node ::toHTML(std::ostream &os) const {
  Tag row(os, "tr");
  { Tag row(os, "td", name); }
  Tag row2(os, "td");
  for (auto &addr : addresses)
    os << Tag::indent() << addr.toString() << "<br>" << std::endl;
}

Node ::Nodes Node ::nodes;
std::unordered_map<std::string, std::string> Node ::ip2hostname;
const Node *Node ::get(std::string hostname) {
  return &(*nodes.find(hostname));
}

bool Node ::addNode(const Node &node) {
  if (nodes.count(node))
    return false;
  nodes.insert(node);
  return true;
}

const Node ::Nodes &Node ::getNodes() { return nodes; }

Node ::SortedNodes Node ::getNodesSorted() {
  SortedNodes sn;
  sn.insert(nodes.begin(), nodes.end());
  return sn;
}

bool operator==(const Node &lhs, const Node &rhs) {
  return lhs.name == rhs.name;
}

bool operator!=(const Node &lhs, const Node &rhs) { return !(lhs == rhs); }

bool operator<(const Node &lhs, const Node &rhs) { return lhs.name < rhs.name; }
