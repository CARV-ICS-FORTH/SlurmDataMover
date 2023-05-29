#pragma once

#include <Poco/Net/TCPServerConnectionFactory.h>

class NodeServerHandler : public Poco::Net::TCPServerConnection {
public:
  NodeServerHandler(const Poco::Net::StreamSocket &s);
  void run();
};
