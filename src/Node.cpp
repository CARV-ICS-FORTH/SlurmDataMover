#include "Node.h"

#include "Poco/Net/DNS.h"
#include "Redis.h"
#include "Utils.h"
#include <iostream>

using Poco::Net::DNS;
using Poco::Net::HostEntry;

Node ::Node(const std::string name) : name(name), addresses() {}

void Node ::resolve() {
  const HostEntry &entry = DNS::thisHost();

  const auto &addrs = entry.addresses();

  addresses.reserve(addrs.size());

  for (auto &addr : addrs) {
    addresses.emplace_back(addr);
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

Node &Node ::getLocalhostNode() {
  static Node localhost(getHostname());
  if (localhost.addresses.size() == 0) {
    localhost.resolve();
    // add to redis
  }
  return localhost;
}

Poco::Net::IPAddress Node ::getIpAddress() const { return addresses.at(0); }

const Node Node ::getFromIp(std::string ip) {
  if (ip == "127.0.0.1")
    return getLocalhostNode();

  return Redis::getNodeByIp(ip);
}

Node ::operator bool() const { return *this == NotFound; }

const Node Node::NotFound("NodeNotFound");

void Node ::toHTML(std::ostream &os) const {
  Tag row(os, "tr");
  { Tag row(os, "td", name); }
  {
    Tag row2(os, "td");
    for (auto &mount : mounts)
      os << Tag::indent() << mount.first << " : " << mount.second << "<br>"
         << std::endl;
  }
  {
    Tag row2(os, "td");
    for (auto &addr : addresses)
      os << Tag::indent() << addr.toString() << "<br>" << std::endl;
  }
}

bool operator==(const Node &lhs, const Node &rhs) {
  return lhs.name == rhs.name;
}

bool operator!=(const Node &lhs, const Node &rhs) { return !(lhs == rhs); }

bool operator<(const Node &lhs, const Node &rhs) { return lhs.name < rhs.name; }
