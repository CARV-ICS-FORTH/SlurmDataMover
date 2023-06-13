#pragma once

#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnectionFactory.h"

class BulkServer : public Poco::Net::TCPServer {
  struct Transfer : Poco::Net::TCPServerConnection {
    Transfer(const Poco::Net::StreamSocket &s);
    void run();
  };

public:
  BulkServer(uint16_t port);
};
