#pragma once
#include "Poco/Any.h"
#include "Poco/Logger.h"
#include <iostream>
#include <string>

class Tag {
public:
  Tag(std::ostream &os, std::string value, std::string inner = "")
      : os(os), tag(value.substr(0, value.find(' '))) {
    os << indentation << "<" << value << ">" << std::endl;
    indentation += "  ";
    if (inner != "")
      os << indentation << inner << std::endl;
  }
  static std::string indent() { return indentation; }
  ~Tag() {
    indentation = indentation.substr(2);
    os << indentation << "</" << tag << ">" << std::endl;
  }

private:
  std::ostream &os;
  std::string tag;
  static std::string indentation;
};

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

private:
  static Poco::Logger *logger;
};