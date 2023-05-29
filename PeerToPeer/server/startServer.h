#ifndef STARTSERVER_H_
#define STARTSERVER_H_
#include <string>
#include <thread>
std::thread *  StartTheServer(std::string ServerHostName, std::string ServerPort);

#endif /*STARTSERVER_H_*/
