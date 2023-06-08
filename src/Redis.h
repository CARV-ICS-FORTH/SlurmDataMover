#pragma once

#include "Node.h"

struct Redis {
  static bool pingRedis();
  static Node::Nodes getAllNodes();
  static Node getNodeByHostname(std::string hostname);
  static Node getNodeByIp(std::string ip);
  static bool addNode(const Node &node);
};
