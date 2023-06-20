#pragma once

#include "File.h"
#include "JSONable.h"
#include "Node.h"
#include <Poco/Net/SocketAddress.h>

namespace Redis {
void connect(Poco::Net::SocketAddress &sock_addr);
bool pingRedis();
Node::Nodes getAllNodes();
Node getNodeByHostname(std::string hostname);

File::Files getAllFiles();
File getFile(std::string name);

/**
 * Place a request for the \c files.
 *
 * \param files Files to be requested
 * \param port Port used in this host to accept the files.
 */
void requestFiles(std::vector<std::string> files, uint16_t port);

bool getRequest();

bool add(const JSONable &node);

bool get(JSONable &node);
}; // namespace Redis
