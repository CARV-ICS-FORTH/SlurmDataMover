#pragma once

#include "File.h"
#include "JSONable.h"
#include "Node.h"
#include <Poco/Net/SocketAddress.h>

namespace Redis {
void connect(const std::string &host, uint16_t port);
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

bool getRequest(std::string &file, std::string &host, uint16_t &port);

bool add(const JSONable &node);

bool get(JSONable &node);

void addLog(const JSONable &obj);

std::vector<std::string> getLastLogs(int count);

}; // namespace Redis
