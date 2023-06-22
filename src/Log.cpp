#include "Log.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Redis.h"
#include <sstream>

using Poco::JSON::Object;
using Poco::JSON::Parser;

Poco::Logger *Log ::logger = 0;

void Log ::setLoggger(Poco::Logger &logger) { Log::logger = &logger; }

std::string sco_str(std::string scope, std::string level) {
  std::stringstream ss;
  ss << "[";
  ss.width(4);
  ss << scope << "] -";
  ss.width(6);
  ss << level << " :: ";
  return ss.str();
}

void Log ::Info(std::string scope, std::string msg) {
  Log::logger->information(sco_str(scope, __func__) + msg);
  Redis::addLog(Entry(scope, __func__, Node::getHostname(), msg));
}

void Log ::Trace(std::string scope, std::string msg) {
  Log::logger->trace(sco_str(scope, __func__) + msg);
  Redis::addLog(Entry(scope, __func__, Node::getHostname(), msg));
}

void Log ::Error(std::string scope, std::string msg) {
  Log::logger->error(sco_str(scope, __func__) + msg);
  Redis::addLog(Entry(scope, __func__, Node::getHostname(), msg));
}

std::vector<Log::Entry> Log ::getLastEntries(int count) {
  std::vector<Log::Entry> entries;
  entries.reserve(count);
  auto rr = Redis::getLastLogs(count);
  for (auto json : rr) {
    Log::Entry temp(json);
    entries.emplace_back(temp);
  }
  return entries;
}

Log::Entry::Entry(std::string json) { fromJSON(json); }

Log::Entry::Entry(std::string scope, std::string type, std::string host,
                  std::string msg)
    : scope(scope), type(type), host(host), msg(msg) {}

std::string Log::Entry::getKey() const { return "log"; }

std::string Log::Entry::toJSON() const {
  Object obj;

  obj.set("host", host);
  obj.set("scope", scope);
  obj.set("msg", msg);
  obj.set("type", type);
  std::stringstream ss;
  obj.stringify(ss);
  return ss.str();
}

void Log::Entry::fromJSON(const std::string &json) {
  Parser parser;
  Object::Ptr obj = parser.parse(json).extract<Object::Ptr>();

  host = obj->getValue<std::string>("host");
  scope = obj->getValue<std::string>("scope");
  msg = obj->getValue<std::string>("msg");
  type = obj->getValue<std::string>("type");
}

void Log::Entry::toHTML(std::ostream &os) const {
  Tag row(os, "tr");
  {
    { Tag(os, "td", host); }
    { Tag(os, "td", type); }
    { Tag(os, "td", scope); }
    { Tag(os, "td", msg); }
  }
}