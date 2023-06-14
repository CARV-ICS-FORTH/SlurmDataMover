#include "Bulk.h"

BulkReciever::Transfer::Transfer(const Poco::Net::StreamSocket &s)
    : Poco::Net::TCPServerConnection(s) {}
void BulkReciever::Transfer::run() {}

BulkReciever::BulkReciever(uint16_t port)
    : Poco::Net::TCPServer(
          new Poco::Net::TCPServerConnectionFactoryImpl<Transfer>(), port) {}

BulkSender::Transfer::Transfer(const Poco::Net::StreamSocket &s)
    : Poco::Net::TCPServerConnection(s) {}
void BulkSender::Transfer::run() {}

BulkSender::BulkSender(uint16_t port)
    : Poco::Net::TCPServer(
          new Poco::Net::TCPServerConnectionFactoryImpl<Transfer>(), port) {}
