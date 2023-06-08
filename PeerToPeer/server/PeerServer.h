/*
 * Peer.h
 *
 *  Created on: 22-May-2023
 *      Author: Klodjan Hidri
 */

#ifndef PEERSERVER_H_
#define PEERSERVER_H_
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>

class PeerServer {

private:
  std::string ServerHostName = "";
  std::string ServerPort = "";
  int sockfd = 0, newsockfd = 0, portno = 0;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  char buffer[256] = {0};
  void SendDataFileToClient(char *, int);
  int GetFilesize(FILE *);
  void newConnection(int sock);
  void SendFileNameAndLengthToClient(char *file_name, int SOCKET, FILE *fp);
  FILE *CreateFileDescriptor(char *file_name);

public:
  PeerServer(std::string, std::string);
  void StartServer();
};

#endif /* PEERSERVER_H_ */
