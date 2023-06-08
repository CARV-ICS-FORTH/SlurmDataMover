#ifndef GETDATA_H_
#define GETDATA_H_

#include "PeerClient.h"
#include <iostream>
#include <string>
#include <vector>

void getDataFile(std::string PeerServerHostName, int PeerServerPort,
                 std::vector<std::string> files);

/*
void getDataFile(std::string PeerServerHostName, int PeerServerPort,
std::vector<std::string>files){
        //std::unique_ptr<PeerClient> recieve(new PeerClient(PeerServerHostName,
PeerServerPort));
        PeerClient *recieve = new PeerClient(PeerServerHostName,
PeerServerPort);

        recieve->ConnectWithServer(PeerServerHostName, PeerServerPort, files);
}
*/

#endif /*GETDATA_H_*/
