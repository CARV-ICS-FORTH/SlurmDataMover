#pragma once
#include "Poco/Net/StreamSocket.h"
#include <iostream>
#include <string>

void sdm_pack(Poco::Net::StreamSocket &os, const std::string &string);
std::string sdm_unpack(Poco::Net::StreamSocket &is);

void sdm_pack_size(Poco::Net::StreamSocket &os, const size_t &size);
size_t sdm_unpack_size(Poco::Net::StreamSocket &is);

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
