/*
 * PeerSender.cpp
 *
 *  Created on: 22-May-2023
 *      Author: Klodjan Hidri
 */

#include "PeerClient.h"
#include "../../src/Utils.h"
#include <errno.h>
#include <vector>

// using namespace std;

void PeerClient::RegisterPeer(std::string hostname, int portno) {
  this->ClientHostName = hostname;
  this->ClientPort = portno;
}

PeerClient::PeerClient(std::string hostname, int portno) {
  this->ClientHostName = hostname;
  this->ClientPort = portno;
}

void PeerClient::ConnectWithServer(std::string hostname, int portno,
                                   std::vector<std::string> files) {
  RegisterPeer(hostname, portno);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    Log::Error("Peer", " Opening Socket");

  server = gethostbyname(ClientHostName.c_str());
  if (server == NULL) {
    Log::Error("Peer", " No Such Host");
    Log::Info("Peer", ": Close Client Connection");
    exit(-1);
  }

  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
  serv_addr.sin_port = htons(ClientPort);
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    Log::Error("Peer", " On Connecting");

  for (std::string file : files) { // listing of files on server
    // send to server node file name and It`s path where is located
    if (send(sockfd, file.c_str(), MAX_COMMAND_LEN, 0) < 0) {
      Log::Error("Peer", " getfile:send_request: Sending Error");
    }
    ReceiveFilefromServer(sockfd, file);
  }

  if (send(sockfd, "exit", MAX_PACKET_CHUNK_LEN, 0) < 0) {
    Log::Error("Peer", "Failed Sending [exit] Command to Server");
  }
  close(sockfd);
}

void PeerClient::GetFileLengthFromServer(std::string file_name,
                                         int &file_data_len, int SOCKET) {

  char buffer[MAX_BUFFER_LEN + 4];
  memset(buffer, 0, sizeof(buffer));

  /* Get File length, under 256 characters message*/
  if (recv(SOCKET, buffer, MAX_PACKET_CHUNK_LEN, 0) < 0) {
    Log::Error("Peer", " Not Reading File Size: Close Client Connection");
    exit(-1);
  }

  file_data_len = atoi(buffer);
  Log::Info("Peer", "Get: %s from %s SIZE: %lu B - Start", file_name,
            ClientHostName, file_data_len);
}

FILE *PeerClient::CreateFileDescriptor(char *file_name) {

  FILE *fp = fopen(file_name, "wb+");
  if (fp == NULL) {
    Log::Error("Peer", " File open error");
    exit(-1);
  }
  return fp;
}

void PeerClient::CheckFileValidation(int received, int file_data_len) {

  if (received >= file_data_len) {
    Log::Info("Peer", "File Recieved From %s Length: %lu B - END",
              ClientHostName, file_data_len);
  } else {
    Log::Error("Peer", "File Length not matching (expected %lu, recieved: %lu)",
               file_data_len, received);
    exit(-1);
  }
}

void PeerClient::ReceiveFilefromServer(int SOCKET, std::string file_name) {
  char buffer[MAX_BUFFER_LEN + 4];
  int file_data_len;

  GetFileLengthFromServer(file_name, file_data_len, SOCKET);

  /* Create File */
  FILE *fp = CreateFileDescriptor(const_cast<char *>(file_name.c_str()));

  int received = 0;
  while (received < file_data_len) {
    int R = recv(SOCKET, buffer, MAX_PACKET_CHUNK_LEN, 0);
    if (R < 0) {
      Log::Error("Peer", "failed recv()");
    }
    if (!fputs(buffer, fp)) {
      Log::Error("Peer", "failed fputs()");
    };
    memset(buffer, 0, sizeof(buffer));
    received += R;
  }

  CheckFileValidation(received, file_data_len);
  fclose(fp);
}

/*
 * PeerClient.cpp
 */
