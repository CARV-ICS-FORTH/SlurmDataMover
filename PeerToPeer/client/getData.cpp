#include "getData.h"
#include "PeerClient.h"
#include <iostream>
#include <vector>
#include <string>

void getDataFile(std::string PeerServerHostName, int PeerServerPort,
                 std::vector<std::string> files) {
  // std::unique_ptr<PeerClient> recieve(new PeerClient(PeerServerHostName,
  // PeerServerPort));
  PeerClient *recieve = new PeerClient(PeerServerHostName, PeerServerPort);
  recieve->ConnectWithServer(PeerServerHostName, PeerServerPort, files);
}
