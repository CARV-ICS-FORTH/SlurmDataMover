#include "Utils.h"
#include <sstream>

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
  size_t len = string.size();
  Log::Trace(__func__, "%lu :: '%s'", len, string);
  sendBytes(sock, (char *)&len, sizeof(size_t));
  sendBytes(sock, &(string.at(0)), len);
}

std::string sdm_unpack(Poco::Net::StreamSocket &sock) {
  size_t len;
  receiveBytes(sock, (char *)&len, sizeof(size_t));
  std::string value(len, 0);
  receiveBytes(sock, &(value.at(0)), len);
  Log::Trace(__func__, "%lu :: '%s'", len, value);
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

Poco::Logger *Log ::logger = 0;

void Log ::setLoggger(Poco::Logger &logger) { Log::logger = &logger; }

std::string sco_str(std::string scope, std::string level) {
  std::stringstream ss;
  ss << "[";
  ss.width(5);
  ss << scope << "] -";
  ss.width(6);
  ss << level << " :: ";
  return ss.str();
}

void Log ::Info(std::string scope, std::string msg) {
  Log::logger->information(sco_str(scope, __func__) + msg);
}

void Log ::Trace(std::string scope, std::string msg) {
  Log::logger->trace(sco_str(scope, __func__) + msg);
}

void Log ::Error(std::string scope, std::string msg) {
  Log::logger->error(sco_str(scope, __func__) + msg);
}
