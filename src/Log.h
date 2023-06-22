#pragma once

#include "JSONable.h"
#include "Poco/Logger.h"

struct Log {
  static void setLoggger(Poco::Logger &logger);

  static void Info(std::string scope, std::string msg);

  template <typename T, typename... Args>
  static void Info(std::string scope, const std::string &fmt, T arg1,
                   Args &&...args) {
    Log::Info(scope, Poco::format(fmt, arg1, std::forward<Args>(args)...));
  }

  static void Trace(std::string scope, std::string msg);

  template <typename T, typename... Args>
  static void Trace(std::string scope, const std::string &fmt, T arg1,
                    Args &&...args) {
    Log::Trace(scope, Poco::format(fmt, arg1, std::forward<Args>(args)...));
  }

  static void Error(std::string scope, std::string msg);

  template <typename T, typename... Args>
  static void Error(std::string scope, const std::string &fmt, T arg1,
                    Args &&...args) {
    Log::Error(scope, Poco::format(fmt, arg1, std::forward<Args>(args)...));
  }

  struct Entry : public JSONable {
    Entry(std::string json);
    Entry(std::string scope, std::string type, std::string host,
          std::string msg);
    std::string scope;
    std::string type;
    std::string host;
    std::string msg;
    void toHTML(std::ostream &os) const;
    IMPLEMENTS_JSONable;
  };

  static std::vector<Entry> getLastEntries(int count = 100);

private:
  static Poco::Logger *logger;
};
