#pragma once

#include "File.h"
#include "JSONable.h"
#include "Node.h"

namespace Redis {
void connect(std::string hostname, uint16_t port);
bool pingRedis();
Node::Nodes getAllNodes();
Node getNodeByHostname(std::string hostname);
Node getNodeByIp(std::string ip);

File::Files getAllFiles();
File getFile(std::string name);

bool add(const JSONable &node);
}; // namespace Redis
