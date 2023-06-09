#pragma once

#include <string>

#define _JSONable_API(ABSTRACT)                                                \
  virtual std::string getKey() const ABSTRACT;                                 \
  virtual std::string toJSON() const ABSTRACT;                                 \
  virtual void fromJSON(const std::string &json) ABSTRACT

#define IMPLEMENTS_JSONable _JSONable_API()

struct JSONable {
  _JSONable_API(= 0);
};
