#include "BulkServer.h"

BulkServer::Transfer::Transfer(const Poco::Net::StreamSocket &s)
    : Poco::Net::TCPServerConnection(s) {}
void BulkServer::Transfer::run() {}

BulkServer::BulkServer(uint16_t port)
    : Poco::Net::TCPServer(
          new Poco::Net::TCPServerConnectionFactoryImpl<Transfer>(), port) {}
