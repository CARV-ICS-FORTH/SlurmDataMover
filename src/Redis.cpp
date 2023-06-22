#include "Redis.h"
#include "Log.h"
#include "Poco/Exception.h"
#include "Poco/Net/NetException.h"
#include "Poco/Redis/Array.h"
#include "Poco/Redis/Client.h"
#include "Poco/Redis/Command.h"
#include "Poco/Redis/Type.h"
#include "Poco/ThreadLocal.h"

using Poco::Redis::Array;
using Poco::Redis::BulkString;
using Poco::Redis::Client;
using Poco::Redis::Command;
using Poco::Redis::RedisType;
using Poco::Redis::Type;

class RedisConnector {
  Poco::ThreadLocal<Client> client;

public:
  std::vector<Poco::Net::SocketAddress> sock_addr = {
      Poco::Net::SocketAddress("redis", 6379)};
  Client *operator->() {
    auto &con = client.get();
    if (!con.isConnected()) {
      for (auto itr = sock_addr.rbegin(); itr != sock_addr.rend(); itr++) {
        auto &redis_sa = *itr;
        try {
          con.connect(redis_sa);
          Log::Info("Redi", "Connected to %s", redis_sa.toString());
          break;
        } catch (Poco::Exception &e) {
          std::cerr << "Redis::connect(" << redis_sa.host() << ":"
                    << redis_sa.port() << "): " << e.displayText() << std::endl;
        }
      }
    }
    if (!con.isConnected())
      throw Poco::Net::NetException("Could not connect to Redis");
    return &con;
  }
} con;

void Redis ::connect(Poco::Net::SocketAddress &sock_addr) {
  con.sock_addr.push_back(sock_addr);
}

bool Redis ::pingRedis() {
  Command ping = Command::ping();

  std::string ret = con->execute<std::string>(ping);

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

      Array response = con->execute<Array>(scan);

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

      Array results = con->execute<Array>(mget);

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

      Array response = con->execute<Array>(scan);

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

        Array results = con->execute<Array>(mget);

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
  BulkString response = con->execute<BulkString>(get);

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
    con->execute<BulkString>(get);
  }
}

bool Redis::getRequest(std::string &file, std::string &host, uint16_t &port) {
  try {
    Array get;
    get << "XREAD"
        << "COUNT"
        << "1"
        << "BLOCK"
        << "100"
        << "STREAMS"
        << "request"
        << "$";
    Array ret = con->execute<Array>(get)
                    .get<Array>(0)
                    .get<Array>(1)
                    .get<Array>(0)
                    .get<Array>(1);
    host = ret.get<BulkString>(1).value();
    port = std::atoi(ret.get<BulkString>(3).value().c_str());
    file = ret.get<BulkString>(5).value();
    return true;
  } catch (Poco::Exception &e) {
    return false;
  }
}

bool Redis::add(const JSONable &obj) {
  try {
    Array cmd;
    cmd << "SET" << obj.getKey() << obj.toJSON();

    std::string ret = con->execute<std::string>(cmd);
  } catch (Poco::Exception &e) {
    std::cerr << "In Redis::add: " << e.displayText();
  }
  return true;
}

bool Redis::get(JSONable &obj) {
  try {
    Array cmd;
    cmd << "GET" << obj.getKey();

    std::string ret = con->execute<BulkString>(cmd);

    obj.fromJSON(ret);
  } catch (Poco::Exception &e) {
    std::cerr << "In Redis::get: " << e.displayText();
  }
  return true;
}

void Redis::addLog(const JSONable &obj) {
  try {
    Array cmd;
    cmd << "LPUSH" << obj.getKey() << obj.toJSON();
    int64_t ret = con->execute<int64_t>(cmd);
  } catch (Poco::Exception &e) {
    std::cerr << "In Redis::addLog: " << e.displayText();
  }
}

std::vector<std::string> Redis::getLastLogs(int count) {
  Command llen = Command::llen("log");
  Poco::Int64 len = con->execute<Poco::Int64>(llen);

  Command range = Command::lrange("log", len - count, len);
  Array ret = con->execute<Array>(range);

  std::vector<std::string> result;

  for (auto &elem : ret)
    result.push_back(getRaw<BulkString>(elem));

  return result;
}
