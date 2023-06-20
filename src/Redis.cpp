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

void Redis ::connect(Poco::Net::SocketAddress &sock_addr) {
  try {
    client.connect(sock_addr);
  } catch (Poco::Exception &e) {
    std::cerr << "In Redis::connect(" << sock_addr.host() << ":"
              << sock_addr.port() << "): " << e.displayText() << std::endl;
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

File Redis::getFile(std::string name) {
  Array get;
  get << "GET"
      << "file:" + name;
  BulkString response = client.execute<BulkString>(get);

  if (response.isNull())
    return File::NotFound;
  else {
    File ret("");
    ret.fromJSON(response);
    return ret;
  }
}

void Redis::requestFiles(std::vector<std::string> files, uint16_t port) {
  std::string sport = std::to_string(port);
  for (auto file : files) {
    Array get;
    get << "XADD"
        << "request"
        << "*"
        << "host" << Node::getHostname() << "port" << sport << "file" << file;
    client.execute<BulkString>(get);
  }
}

bool Redis::getRequest() {
  Array get;
  get << "XREAD"
      << "COUNT"
      << "1"
      << "BLOCK"
      << "0"
      << "STREAMS"
      << "request"
      << "$";
  RedisType::Ptr ret = client.sendCommand(get);
  if (ret) {
    if (!ret->isArray())
      return false;
    Array *arr = (Array *)ret.get();
    std::cerr << arr->toString();
  }
  return ret;
}

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

bool Redis::get(JSONable &obj) {
  try {
    Array cmd;
    cmd << "GET" << obj.getKey();

    std::string ret = client.execute<BulkString>(cmd);

    obj.fromJSON(ret);
  } catch (Poco::Exception &e) {
    std::cerr << "In Redis::get: " << e.displayText();
  }
  return true;
}
