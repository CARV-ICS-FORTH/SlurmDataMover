#pragma once

#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include <map>

class BulkReciever : public Poco::Net::TCPServer {
  struct Transfer : Poco::Net::TCPServerConnection {
    Transfer(const Poco::Net::StreamSocket &s);
    void run();
  };

public:
  typedef std::map<std::string, std::string> FileList;
  BulkReciever(FileList files, uint16_t port = 0);
  void wait();

private:
  FileList files;
  Poco::Event e;
};

class BulkSender : public Poco::Net::TCPServer, public Poco::Thread {
  struct Transfer : Poco::Net::TCPServerConnection {
    Transfer(const Poco::Net::StreamSocket &s);
    void run();
  };
  bool _stop;

public:
  BulkSender(uint16_t port = 0);
  void run();
  void stop();
};
