#include "Node.h"
#include "Poco/Exception.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Net/DNS.h"
#include "Poco/Path.h"
#include "Redis.h"
#include "Utils.h"
#include <iostream>
#include <regex>

using Poco::JSON::Array;
using Poco::JSON::Object;
using Poco::JSON::Parser;
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

bool Node ::parseMountPath(const std::string &mount_path, std::string &file,
                           std::string &mount) {
  static std::regex mount_fmt("([^@]+)@([^@]+)");
  std::smatch match;
  if (!std::regex_match(mount_path, match, mount_fmt)) {
    std::cerr << "Improper mount format for \'" << mount_path << "\'"
              << std::endl;
    return false;
  }

  file = match[1];
  mount = match[2];

  return true;
}

bool Node::parseMountPath(const std::string &mount_path, std::string &file) {
  std::string temp;
  return parseMountPath(mount_path, file, temp);
}

std::string Node ::normalizeMountPath(std::string mount_path) {
  std::string file;
  std::string mount;
  if (!parseMountPath(mount_path, file, mount))
    return "";
  if (!mounts.count(mount))
    throw Poco::PathNotFoundException("Mount not found", mount);
  return Poco::Path(mounts.at(mount)).append(file).toString();
}

Node ::operator bool() const { return *this == NotFound; }

std::string Node ::getKey() const { return "node:" + name; }

std::string Node ::toJSON() const {
  Object obj;
  Array ips;

  obj.set("name", name);

  for (auto addr : addresses)
    ips.add(addr.toString());

  obj.set("addresses", ips);

  Object mounts;

  for (auto mount : this->mounts)
    mounts.set(mount.first, mount.second);

  obj.set("mounts", mounts);

  std::stringstream ss;
  obj.stringify(ss);
  return ss.str();
}

void Node ::fromJSON(const std::string &json) {
  Parser parser;
  Object::Ptr obj = parser.parse(json).extract<Object::Ptr>();

  name = obj->getValue<std::string>("name");

  Array::Ptr ips = obj->getArray("addresses");

  addresses.clear();
  for (auto ip : (*ips))
    addresses.emplace_back(ip.toString());

  Object::Ptr mounts = obj->getObject("mounts");
  this->mounts.clear();
  for (auto mount : (*mounts))
    this->mounts.insert(
        std::make_pair(mount.first, mount.second.convert<std::string>()));
}

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
