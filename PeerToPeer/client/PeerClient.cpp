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

using namespace std;

void PeerClient::RegisterPeer(string hostname, int portno){
	this->ClientHostName=hostname;
	this->ClientPort=portno;
}

 PeerClient::PeerClient(string hostname, int portno){
   	cout<<"Constructor\n";
	this->ClientHostName=hostname; 
	this->ClientPort=portno;
}



void PeerClient::ConnectWithServer(string hostname, int portno,vector<string> files){
	RegisterPeer(hostname, portno);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		cerr<< "ERROR opening socket\n";

	server = gethostbyname(ClientHostName.c_str());
	if (server == NULL) {
		cerr<< "ERROR, no such host\n";
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr,(char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(ClientPort);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		cerr<< "ERROR connecting" << strerror(errno) << "\n";

	char cmd[MAX_COMMAND_LEN];
            
        for(string file: files){ //listing of server files
        	strcpy(cmd, file.c_str()); 
		// send to server node file name and It`s path where is located  
		if (send(sockfd, cmd, MAX_COMMAND_LEN, 0) < 0) {
			cerr << "getfile:send_request: Sending Error\n";
		}
		ReceiveFilefromServer(sockfd);
	}

	if (send(sockfd, "exit", MAX_PACKET_CHUNK_LEN, 0) < 0) {
		cerr <<"Error send while exiting\n";
	}
	close(sockfd);
        cout<<"Closed Connection With Server Node ["<<ClientHostName<<"]\n";
}


void PeerClient::GetFileNameAndLengthFromServer( char file_name[],
						int &file_data_len, int SOCKET){


	char buffer[MAX_BUFFER_LEN + 4];
        memset(buffer, 0, sizeof(buffer));
	

	/* Get File name + len, under 256 characters */
	if (recv(SOCKET, buffer, MAX_PACKET_CHUNK_LEN, 0) < 0) {
		cerr<<"ERROR: Reading file name\n";
		return;
	}
	printf("1: %s\n",buffer);
	char *end_pointer;
	char* ch = strtok_r(buffer, "|", &end_pointer);
	
	strncpy(file_name, ch, strlen(ch));
	strcat(file_name, "\0");
	ch = strtok_r(NULL, " ,", &end_pointer);
	file_data_len = atoi(ch);
	printf("%sFileName: %s%s\n", KRED, RESET, file_name);
	printf("%sFilesize: %s%d bytes\n", KRED, RESET, file_data_len);



}

FILE * PeerClient::CreateFileDescriptor( char file_name[]){
      
       FILE *fp= fopen(file_name, "wb+");
	if (fp == NULL) {
		printf("File open error");
		return 0;
	}
       
      return fp;
}


void PeerClient::CheckFileValidation(int received, int file_data_len){
	
	if (received >= file_data_len) {
		printf("%sFile Length:%s nbytes %s%s%s\n", KRED, RESET, KGRN, TICK,
				RESET);
	} else {
		printf("%sCONCERN:%s File Length not matching%s\n", KRED, KYEL, RESET);
	}


}
void PeerClient::ReceiveFilefromServer(int SOCKET) {
	char buffer[MAX_BUFFER_LEN + 4];
	char file_name[MAX_BUFFER_LEN];
        memset(file_name, 0, sizeof(file_name)); 
	int file_data_len;

	GetFileNameAndLengthFromServer(file_name,
				file_data_len, SOCKET);


	printf("1: %s, SIZE: %d\n",buffer, file_data_len);

	/* Create File */
	FILE *fp= CreateFileDescriptor(file_name); 
	printf("%s-----------------------------------------------------%s\n", KYEL,
			RESET);
	int received = 0;
	while (received < file_data_len) {
		int R = recv(SOCKET, buffer, MAX_PACKET_CHUNK_LEN, 0);
		if (R < 0) {
			cerr<< "Error: While receiving\n";
		}
		if (!fputs(buffer, fp)) {
			cerr<< "ERROR: While saving to file\n";
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
