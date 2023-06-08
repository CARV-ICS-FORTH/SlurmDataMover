#include "startServer.h"
#include "PeerServer.h"
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

std::thread *StartTheServer(std::string ServerHostName,
                            std::string ServerPort) {
  std::thread *th = new std::thread([ServerHostName, ServerPort]() {
    auto ps = new PeerServer(ServerHostName, ServerPort);
    ps->StartServer();
  });
  return th;
}
