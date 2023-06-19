#include "Bulk.h"
#include "Redis.h"
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
          new Poco::Net::TCPServerConnectionFactoryImpl<Transfer>(), port),
      Poco::Thread("Bulk") {}

void BulkSender::run() {
  Log::Info("Bulk", "Ready to send");
  while (1) {
    std::cerr << "Request" << (Redis::getRequest() ? "Yep" : "Nop")
              << std::endl;
  }
}