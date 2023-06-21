#include "Bulk.h"
#include "Poco/FileStream.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/StreamCopier.h"
#include "Redis.h"
#include "Utils.h"

BulkReciever::FileList recv_files;
Poco::Event e;

BulkReciever::Transfer::Transfer(const Poco::Net::StreamSocket &s)
    : Poco::Net::TCPServerConnection(s) {}
void BulkReciever::Transfer::run() {
  auto &sock = socket();
  Poco::Net::SocketStream istr(sock);
  size_t fname_s;
  sock.receiveBytes(&fname_s, sizeof(fname_s));
  char file[fname_s];
  sock.receiveBytes(file, fname_s);

  std::string target = recv_files.at(file);
  Log::Info("Bulk", "Recieve " + std::string(file) + " at " + target);

  Poco::FileOutputStream ostr(target, std::ios::binary);
  Poco::StreamCopier::copyStream(istr, ostr);

  recv_files.erase(file);

  if (recv_files.size() == 0)
    e.set();
}

BulkReciever::BulkReciever(BulkReciever::FileList files, uint16_t port)
    : Poco::Net::TCPServer(
          new Poco::Net::TCPServerConnectionFactoryImpl<Transfer>(), port) {
  recv_files.insert(files.begin(), files.end());
}

void BulkReciever::wait() {
  Log::Info("Bulk", "Waiting for %lu BulkRecieve at %hu", recv_files.size(),
            port());
  start();
  e.wait();
  e.reset();
}

BulkSender::Transfer::Transfer(const Poco::Net::StreamSocket &s)
    : Poco::Net::TCPServerConnection(s) {}
void BulkSender::Transfer::run() {}

BulkSender::BulkSender(uint16_t port)
    : Poco::Net::TCPServer(
          new Poco::Net::TCPServerConnectionFactoryImpl<Transfer>(), port),
      Poco::Thread("Bulk"), _stop(false) {}

void BulkSender::run() {
  Log::Info("Bulk", "Ready to send");
  std::string file;
  std::string host;
  uint16_t port;
  while (!_stop) {
    if (Redis::getRequest(file, host, port)) {
      std::string location = File::Locate(file);
      Log::Info("Bulk", "Request for %s from %s:%hu", file, host, port);
      if (location != "") {
        Log::Info("Bulk", "Send %s to %s:%hu from %s", file, host, port,
                  location);
        Poco::Net::StreamSocket ss(Poco::Net::SocketAddress(host, port));
        Poco::Net::SocketStream ostr(ss);

        size_t fname_s = file.size() + 1;

        ss.sendBytes(&fname_s, sizeof(fname_s));
        ss.sendBytes(file.c_str(), fname_s);

        Poco::FileInputStream istr(location, std::ios::binary);
        Poco::StreamCopier::copyStream(istr, ostr);
        Log::Info("Bulk", "Sent %s to %s:%hu from %s", file, host, port,
                  location);
      }
    }
  }
}

void BulkSender::stop() { _stop = true; }

BulkSender::~BulkSender() { stop(); }
