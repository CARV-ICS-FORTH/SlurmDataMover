#include "Utils.h"
#include "Poco/Util/Application.h"
#include <sstream>

using namespace Poco::Util;

static void sendBytes(Poco::Net::StreamSocket &sock, const char *data,
                      size_t size) {
  while (size) {
    int read = sock.sendBytes(data, size);
    data += read;
    size -= read;
  }
}

static void receiveBytes(Poco::Net::StreamSocket &sock, char *data,
                         size_t size) {
  Poco::Timespan timeOut(10, 0);
  while (size) {
    if (sock.poll(timeOut, Poco::Net::Socket::SELECT_READ)) {
      int read = sock.receiveBytes(data, size);
      data += read;
      size -= read;
    }
  }
}

void sdm_pack(Poco::Net::StreamSocket &sock, const std::string &string) {
  Application &app = Application::instance();
  Poco::Logger &log = app.logger();

  size_t len = string.size();
  log.trace("[%s] %lu :: '%s'", std::string(__func__), len, string);
  sendBytes(sock, (char *)&len, sizeof(size_t));
  sendBytes(sock, &(string.at(0)), len);
}

std::string sdm_unpack(Poco::Net::StreamSocket &sock) {
  Application &app = Application::instance();
  Poco::Logger &log = app.logger();

  size_t len;
  receiveBytes(sock, (char *)&len, sizeof(size_t));
  std::string value(len, 0);
  receiveBytes(sock, &(value.at(0)), len);
  log.trace("[%s] %lu :: '%s'", std::string(__func__), len, value);
  return value;
}

void sdm_pack_size(Poco::Net::StreamSocket &sock, const size_t &size) {
  sendBytes(sock, (char *)&size, sizeof(size_t));
}

size_t sdm_unpack_size(Poco::Net::StreamSocket &sock) {
  size_t temp;
  receiveBytes(sock, (char *)&temp, sizeof(size_t));
  return temp;
}

std::string Tag ::indentation = "";
