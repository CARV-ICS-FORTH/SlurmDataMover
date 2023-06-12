#pragma once

#include "File.h"
#include "JSONable.h"
#include "Node.h"

namespace Redis {
void connect(std::string hostname, uint16_t port);
bool pingRedis();
Node::Nodes getAllNodes();
Node getNodeByHostname(std::string hostname);

File::Files getAllFiles();
File getFile(std::string name);

/**
 * Place a request for the \c files.
 *
 * \param files Files to be requested
 * \param port On this host to accept the files.
 */
void requestFiles(std::vector<std::string> files, uint16_t port);

bool add(const JSONable &node);
}; // namespace Redis
