#include "Redis.h"
#include "Poco/Exception.h"
#include "Poco/Redis/Array.h"
#include "Poco/Redis/Client.h"
#include "Poco/Redis/Command.h"
#include "Poco/Redis/Type.h"

using Poco::Redis::Array;
using Poco::Redis::BulkString;
using Poco::Redis::Client;
using Poco::Redis::Command;
using Poco::Redis::Type;

Client client(Node::getHostname(), 6379);

bool Redis ::pingRedis() {
  Command ping = Command::ping();

  std::string ret = client.execute<std::string>(ping);

  return ret == "PONG";
}

Node::Nodes Redis ::getAllNodes() {
  Node::Nodes ret;

  try {
    Array cmd;
    std::string currsor = "0";
    do {
      cmd << "SCAN" << currsor << "match"
          << "node:*";

      Array response = client.execute<Array>(cmd);

      currsor = std::to_string(response.get<BulkString>(0));

      Array results = response.get<Array>(1);

      if (!results.isNull()) {
        for (auto result : results) {
          auto test = result->toString();
          if (result->isBulkString()) {
            std::string node = ((Type<BulkString> *)result.get())->value();
            std::cerr << "[" << node << "]";
            ret.insert(node.substr(5));
          }
        }
      }

    } while (currsor != "0");
  } catch (Poco::Exception &e) {
    std::cerr << e.displayText();
  }

  return ret;
}

Node Redis ::getNodeByIp(std::string ip) { return Node::NotFound; }

bool Redis::addNode(const Node &node) {
  std::map<std::string, std::string> map;

  map["hello"] = "world";

  try {
    Command set = Command::hmset("node:" + node.name, map);

    std::string ret = client.execute<std::string>(set);
  } catch (Poco::Exception &e) {
    std::cerr << e.displayText();
  }
  return true;
}
