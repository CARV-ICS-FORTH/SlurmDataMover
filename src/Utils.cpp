#include "Utils.h"
#include "Log.h"
#include "Poco/Process.h"
#include "Poco/StringTokenizer.h"
#include <sstream>

Poco::ThreadLocal<std::string> Tag ::indentation;

Timeit ::Timeit(const char *msg) : msg(msg) {}

Timeit ::Timeit(const std::string &msg) : msg(msg) {}

Timeit ::~Timeit() {
  Poco::Timestamp now;
  Log::Info("Time", "%s [%ld,%ld,%ld]", msg, start - boot, now - start,
            now - boot);
}

const Poco::Timestamp Timeit::boot;

int executeProgram(std::string program, std::string cwd) {
  Timeit _t("Execute " + program + " at " + cwd);
  std::vector<std::string> args;

  Poco::StringTokenizer stk(program, " ");
  std::string executable;

  for (auto tok : stk)
    if (executable == "")
      executable = tok;
    else
      args.push_back(tok);

  int ret = Poco::Process::launch(*stk.begin(), args, cwd).wait();

  return ret;
}
