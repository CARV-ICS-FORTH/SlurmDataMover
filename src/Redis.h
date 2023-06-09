#pragma once

#include "File.h"
#include "JSONable.h"
#include "Node.h"

struct Redis {
  static bool pingRedis();
  static Node::Nodes getAllNodes();
  static Node getNodeByHostname(std::string hostname);
  static Node getNodeByIp(std::string ip);

  static File::Files getAllFiles();
  static File getFile(std::string name);

  static bool add(const JSONable &node);
};
