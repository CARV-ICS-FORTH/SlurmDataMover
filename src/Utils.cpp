#include "Utils.h"
#include <sstream>

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

std::string createTempFolder(std::string root) { static int cnt = 0; }
