#include "Utils.h"
#include "Poco/Process.h"
#include "Poco/StringTokenizer.h"
#include <sstream>

std::string Tag ::indentation = "";

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
}

void Log ::Trace(std::string scope, std::string msg) {
  Log::logger->trace(sco_str(scope, __func__) + msg);
}

void Log ::Error(std::string scope, std::string msg) {
  Log::logger->error(sco_str(scope, __func__) + msg);
}

int executeProgram(std::string program, std::string cwd) {
  std::vector<std::string> args;

  Poco::StringTokenizer stk(program, " ");
  std::string executable;

  for (auto tok : stk)
    if (executable == "")
      executable = tok;
    else
      args.push_back(tok);

  return Poco::Process::launch(*stk.begin(), args, cwd).wait();
}
