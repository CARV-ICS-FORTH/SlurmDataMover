/*
 * PeerSender.cpp
 *
 *  Created on: 22-May-2023
 *      Author: Klodjan Hidri
 */

#ifndef PEERCLIENT_H_
#define PEERCLIENT_H_
#include "../color.h"
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
// #include <thread>
#include <csignal>
#include <iostream>
#include <signal.h>

#include <sys/wait.h>
#include <unistd.h>

class PeerClient {
private:
  std::string ClientHostName = "";
  int ClientPort = 0;
  int sockfd = 0, portno = 0;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[256] = {0};
  char ip[INET_ADDRSTRLEN] = {0};
  unsigned char *receive = NULL;

  void RegisterPeer(std::string hostname, int portno);
  void GetFileLengthFromServer(std::string file_name, int &file_data_len,
                               int SOCKET);
  FILE *CreateFileDescriptor(char *file_name);
  void CheckFileValidation(int received, int file_data_len);
  void ReceiveFilefromServer(int SOCKET, std::string file_name);

public:
  PeerClient(std::string, int);
  void ConnectWithServer(std::string hostname, int portno,
                         std::vector<std::string> files);
};

#endif /* PEERCLIENT_H_ */
