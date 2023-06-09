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
using Poco::Redis::RedisType;
using Poco::Redis::Type;

Client client(Node::getHostname(), 6379);

bool Redis ::pingRedis() {
  Command ping = Command::ping();

  std::string ret = client.execute<std::string>(ping);

  return ret == "PONG";
}

template <typename T> T getRaw(const RedisType *rdt) {
  return ((Type<BulkString> *)rdt)->value();
}

Node::Nodes Redis ::getAllNodes() {
  Node::Nodes ret;

  try {
    Array scan;
    std::string currsor = "0";
    std::vector<std::string> node_keys;
    do {
      scan << "SCAN" << currsor << "match"
           << "node:*";

      Array response = client.execute<Array>(scan);

      currsor = std::to_string(response.get<BulkString>(0));

      Array results = response.get<Array>(1);

      if (!results.isNull()) {
        for (auto result : results) {
          auto test = result->toString();
          if (result->isBulkString()) {
            node_keys.push_back(getRaw<BulkString>(result.get()));
          }
        }
      }
    } while (currsor != "0");

    {
      Array mget;
      mget << "MGET";
      for (auto &nk : node_keys)
        mget << nk;

      Array results = client.execute<Array>(mget);

      if (!results.isNull()) {
        for (auto result : results) {
          Node temp("");
          temp.fromJSON(getRaw<BulkString>(result));
          ret.insert(temp);
        }
      }
    }
  } catch (Poco::Exception &e) {
    std::cerr << e.displayText();
  }

  return ret;
}

Node Redis ::getNodeByIp(std::string ip) { return Node::NotFound; }

bool Redis::addNode(const Node &node) {
  try {
    Array cmd;
    cmd << "SET" << node.getKey() << node.toJSON();

    std::string ret = client.execute<std::string>(cmd);
  } catch (Poco::Exception &e) {
    std::cerr << e.displayText();
  }
  return true;
}
