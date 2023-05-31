#pragma once

#include "Node.h"
#include "Utils.h"

#include <ostream>
#include <string>
#include <unordered_set>

struct File {
  std::unordered_set<std::string> nodes;
  std::string file_name;
  std::string location;
  size_t size;
  File(const std::string &file_name);
  void toHTML(std::ostream &os) const;
  void updateFrom(const File &update);

  typedef std::unordered_set<File> FileState;
  static FileState files;
};

bool operator==(const File &lhs, const File &rhs);
Poco::Net::StreamSocket &operator<<(Poco::Net::StreamSocket &sock,
                                    const File &file);
Poco::Net::StreamSocket &operator>>(Poco::Net::StreamSocket &sock, File &file);

namespace std {
template <> struct hash<File> {
  size_t operator()(const File file) const {
    return std::hash<std::string>()(file.file_name);
  }
};
} // namespace std
