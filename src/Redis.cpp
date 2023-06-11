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

Client client;

void Redis ::connect(std::string host, uint16_t port) {
  try {
    client.connect(host, port);
  } catch (Poco::Exception &e) {
    std::cerr << "In Redis::connect(" << host << ":" << port
              << "): " << e.displayText() << std::endl;
    throw e;
  }
}

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
    std::cerr << "In Redis::getAllNodes: " << e.displayText();
  }

  return ret;
}

Node Redis ::getNodeByIp(std::string ip) { return Node::NotFound; }

File::Files Redis ::getAllFiles() {
  File::Files ret;

  try {
    Array scan;
    std::string currsor = "0";
    std::vector<std::string> file_keys;
    do {
      scan << "SCAN" << currsor << "match"
           << "file:*";

      Array response = client.execute<Array>(scan);

      currsor = std::to_string(response.get<BulkString>(0));

      Array results = response.get<Array>(1);

      if (!results.isNull()) {
        for (auto result : results) {
          auto test = result->toString();
          if (result->isBulkString()) {
            file_keys.push_back(getRaw<BulkString>(result.get()));
          }
        }
      }
    } while (currsor != "0");

    if (file_keys.size()) {
      {
        Array mget;
        mget << "MGET";
        for (auto &nk : file_keys)
          mget << nk;

        Array results = client.execute<Array>(mget);

        if (!results.isNull()) {
          for (auto result : results) {
            File temp("");
            temp.fromJSON(getRaw<BulkString>(result));
            ret.insert(temp);
          }
        }
      }
    }
  } catch (Poco::Exception &e) {
    std::cerr << "In Redis::getAllFiles: " << e.displayText();
  }

  return ret;
}

File Redis::getFile(std::string name) { return File::NotFound; }

bool Redis::add(const JSONable &obj) {
  try {
    Array cmd;
    cmd << "SET" << obj.getKey() << obj.toJSON();

    std::string ret = client.execute<std::string>(cmd);
  } catch (Poco::Exception &e) {
    std::cerr << "In Redis::add: " << e.displayText();
  }
  return true;
}
