#pragma once

#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnectionFactory.h"

class BulkReciever : public Poco::Net::TCPServer {
  struct Transfer : Poco::Net::TCPServerConnection {
    Transfer(const Poco::Net::StreamSocket &s);
    void run();
  };

public:
  BulkReciever(uint16_t port = 0);
};

class BulkSender : public Poco::Net::TCPServer {
  struct Transfer : Poco::Net::TCPServerConnection {
    Transfer(const Poco::Net::StreamSocket &s);
    void run();
  };

public:
  BulkSender(uint16_t port = 0);
};
