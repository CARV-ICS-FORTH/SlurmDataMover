#pragma once

#include "Node.h"
#include "Utils.h"

#include <ostream>
#include <string>
#include <unordered_set>

struct File : public JSONable {
  typedef std::unordered_set<File> Files;
  std::unordered_set<std::string> nodes;
  std::string file_name;
  std::string location;
  size_t size;
  File(const std::string &file_name);
  void toHTML(std::ostream &os) const;

  IMPLEMENTS_JSONable;

  static std::string Locate(std::string name);

  static const File NotFound;
};

bool operator==(const File &lhs, const File &rhs);

namespace std {
template <> struct hash<File> {
  size_t operator()(const File file) const {
    return std::hash<std::string>()(file.file_name);
  }
};
} // namespace std
