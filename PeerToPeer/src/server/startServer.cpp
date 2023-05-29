#include "startServer.h"
#include "PeerServer.h"
#include <signal.h>
#include<sys/wait.h>
#include<unistd.h>
#include <iostream>

void StartTheServer(std::string ServerHostName, std::string ServerPort){
	//std::unique_ptr<PeerServer> rec(new PeerServer(ServerHostName,ServerPort));
        PeerServer *server = new PeerServer(ServerHostName,ServerPort);
	server->StartServer();
}

