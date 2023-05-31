/*
 * PeerSender.cpp
 *
 *  Created on: 22-May-2023
 *      Author: Klodjan Hidri
 */

#include "PeerClient.h"
#include "../color.h"
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

void PeerClient::LogError(std::string msg) {
  std::cerr << "ERROR: " << msg << " [" << strerror(errno) << "] @ " << __FILE__
            << ':' << __LINE__ << std::endl;
}

void PeerClient::Log(std::string msg) {
  std::cout << "[" << msg << "]" << std::endl;
}

void PeerClient::ConnectWithServer(std::string hostname, int portno,
                                   std::vector<std::string> files) {
  RegisterPeer(hostname, portno);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    LogError("PeerClient Opening Socket");

  server = gethostbyname(ClientHostName.c_str());
  if (server == NULL) {
    LogError("PeerClient No Such Host");
    Log("PeerClient: Close Client Connection");
    exit(-1);
  }

  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
  serv_addr.sin_port = htons(ClientPort);
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    LogError("PeerClient On Connecting");

  for (std::string file : files) { // listing of files on server
    // send to server node file name and It`s path where is located
    if (send(sockfd, file.c_str(), MAX_COMMAND_LEN, 0) < 0) {
      LogError("PeerClient getfile:send_request: Sending Error");
    }
    ReceiveFilefromServer(sockfd, file);
  }

  if (send(sockfd, "exit", MAX_PACKET_CHUNK_LEN, 0) < 0) {
    LogError("Failed Sending [exit] Command to Server");
  }
  close(sockfd);
}

void PeerClient::GetFileLengthFromServer(std::string file_name,
                                         int &file_data_len, int SOCKET) {

  char buffer[MAX_BUFFER_LEN + 4];
  memset(buffer, 0, sizeof(buffer));

  /* Get File length, under 256 characters message*/
  if (recv(SOCKET, buffer, MAX_PACKET_CHUNK_LEN, 0) < 0) {
    LogError("PeerClient Not Reading File Size: Close Client Connection");
    exit(-1);
  }

  file_data_len = atoi(buffer);
  Log(std::string(KRED) + "Get: " + std::string(RESET) + file_name + " from " +
      ClientHostName + +" SIZE: " + std::string(KRED) + std::string(RESET) +
      std::to_string(file_data_len) + "B - Start");
}

FILE *PeerClient::CreateFileDescriptor(char *file_name) {

  FILE *fp = fopen(file_name, "wb+");
  if (fp == NULL) {
    LogError("PeerClient File open error");
    exit(-1);
  }
  return fp;
}

void PeerClient::CheckFileValidation(int received, int file_data_len) {

  if (received >= file_data_len) {
    Log(std::string(KRED) + "File Recieved From " + ClientHostName +
        " Length:" + std::string(RESET) + std::to_string(file_data_len) +
        std::string(KGRN) + std::string(TICK) + " - END");
  } else {
    Log("CONCERN:" + std::string(KRED) + " File Length not matching" +
        std::string(KYEL) + std::string(RESET));
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
      LogError("While Receiving File Data From Server");
    }
    if (!fputs(buffer, fp)) {
      LogError("While Saving To File Data Received From Server");
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
