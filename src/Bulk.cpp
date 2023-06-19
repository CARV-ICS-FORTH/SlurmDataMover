#include "Bulk.h"
#include "Utils.h"

BulkReciever::Transfer::Transfer(const Poco::Net::StreamSocket &s)
    : Poco::Net::TCPServerConnection(s) {}
void BulkReciever::Transfer::run() {}

BulkReciever::BulkReciever(BulkReciever::FileList files, uint16_t port)
    : Poco::Net::TCPServer(
          new Poco::Net::TCPServerConnectionFactoryImpl<Transfer>(), port),
      files(files) {}

void BulkReciever::wait() {
  Log::Info("Bulk", "Waiting for %lu BulkRecieve", files.size());
  e.wait();
}

BulkSender::Transfer::Transfer(const Poco::Net::StreamSocket &s)
    : Poco::Net::TCPServerConnection(s) {}
void BulkSender::Transfer::run() {}

BulkSender::BulkSender(uint16_t port)
    : Poco::Net::TCPServer(
          new Poco::Net::TCPServerConnectionFactoryImpl<Transfer>(), port) {}
