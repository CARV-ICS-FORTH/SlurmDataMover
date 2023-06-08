#pragma once

#include "Poco/Logger.h"
#include "Poco/Net/SocketStream.h"
#include <string>
#include <unordered_map>

typedef void node_op(Poco::Net::StreamSocket &sock, std::string &aka);

extern std::unordered_map<std::string, node_op *> node_op_map;
