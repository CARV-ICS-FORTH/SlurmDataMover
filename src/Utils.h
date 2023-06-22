#pragma once
#include "Poco/Any.h"
#include "Poco/ThreadLocal.h"
#include "Poco/Timestamp.h"
#include <iostream>
#include <string>

class Tag {
public:
  Tag(std::ostream &os, std::string value, std::string inner = "")
      : os(os), tag(value.substr(0, value.find(' '))) {
    os << indentation.get() << "<" << value << ">" << std::endl;
    indentation.get() += "  ";
    if (inner != "")
      os << indentation.get() << inner << std::endl;
  }
  static std::string indent() { return indentation.get(); }
  ~Tag() {
    indentation.get() = indentation.get().substr(2);
    os << indentation.get() << "</" << tag << ">" << std::endl;
  }

private:
  std::ostream &os;
  std::string tag;
  static Poco::ThreadLocal<std::string> indentation;
};

class Timeit {
  Poco::Timestamp start;
  const std::string msg;

public:
  Timeit(const char *msg);
  Timeit(const std::string &msg);
  ~Timeit();
  static const Poco::Timestamp boot;
};

int executeProgram(std::string program, std::string cwd);
