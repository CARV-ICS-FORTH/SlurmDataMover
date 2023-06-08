/*
 * PeerReceiver.cpp
 *
 *  Created on: 22-May-2023
 *      Author: Klodjan Hidri
 *
 */
#include "PeerServer.h"
#include "../../src/Utils.h"
#include "../color.h"
#include <unistd.h>

PeerServer::PeerServer(std::string ServerHostName, std::string ServerPort) {

  this->ServerHostName = ServerHostName;
  this->ServerPort = ServerPort;
}

void PeerServer::StartServer() {

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    Log::Error("Peer", "Server Opening Socket failed");
  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(ServerPort.c_str());
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    Log::Error("Peer", "Bind socket failed");
  listen(sockfd, 10);
  clilen = sizeof(cli_addr);

  Log::Info("Peer", "Started on Port: " + std::to_string(portno));

  while (1) {
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      Log::Error("Peer", "Accept failed");

    pid_t pid = fork();
    if (pid < 0)
      Log::Error("Peer", "Fork failed");
    if (pid == 0) {
      close(sockfd);
      newConnection(newsockfd);
      exit(0);
    } else
      close(newsockfd);
  }
  close(sockfd);
}

void PeerServer::newConnection(int sock) {
  char cmd[MAX_COMMAND_LEN];

  if (recv(sock, cmd, MAX_COMMAND_LEN, 0) < 0)
    Log::Error("Peer", "Receive FileName failed");

  while (strcmp(cmd, "exit")) {
    SendDataFileToClient(cmd, sock);
    memset(cmd, 0, sizeof(cmd));
    if (recv(sock, cmd, MAX_COMMAND_LEN, 0) < 0)
      Log::Error("Peer", "Receive Command failed");
  }
}

FILE *PeerServer::CreateFileDescriptor(char *file_name) {

  FILE *fp = fopen(file_name, "rb");
  if (fp == NULL) {
    Log::Error("Peer", std::string(file_name) + " could not be opened");
    //_exit(-1);
    return 0;
  }

  if (strlen(file_name) >= MAX_BUFFER_LEN) {
    Log::Error("Peer",
               "Filename too long, max " + std::to_string(MAX_BUFFER_LEN));
    fclose(fp);
    return NULL;
  }
  return fp;
}

void PeerServer::SendFileNameAndLengthToClient(char *file_name, int SOCKET,
                                               FILE *fp) {

  char buff[MAX_BUFFER_LEN + 4];
  memset(buff, 0, sizeof(buff));

  int SIZE = GetFilesize(fp);
  char snum[5];
  sprintf(snum, "%d", SIZE);

  // Write all
  memset(buff, 0, sizeof(buff));
  strcat(buff, snum);
  Log::Info("Peer", "Send File_Length: " + std::string(buff));

  if (strlen(snum) + strlen(file_name) + 1 > MAX_PACKET_CHUNK_LEN) {
    Log::Error("Peer", "Name + Size length exceeded. Error may occur");
  }

  if (send(SOCKET, buff, MAX_PACKET_CHUNK_LEN, 0) < 0)
    Log::Error("Peer", "Send FileSize failed");
}

void PeerServer::SendDataFileToClient(char *file_name, int socket_id) {

  char buff[MAX_BUFFER_LEN + 3];
  memset(buff, 0, sizeof(buff));

  FILE *fp = CreateFileDescriptor(file_name);
  SendFileNameAndLengthToClient(file_name, socket_id, fp);

  int R;
  while ((R = fread(buff, sizeof(char), MAX_PACKET_CHUNK_LEN, fp))) {
    if (send(socket_id, buff, MAX_PACKET_CHUNK_LEN, 0) < 0) {
      Log::Error("Peer", "Send File Data failed");
    };
    bzero(buff, MAX_PACKET_CHUNK_LEN);
  }

  memset(buff, 0, sizeof(buff));
  fclose(fp);
}

int PeerServer::GetFilesize(FILE *fileid) {
  fseek(fileid, 0L, SEEK_END);
  int sz = ftell(fileid);
  fseek(fileid, 0L, SEEK_SET);
  return sz;
}
