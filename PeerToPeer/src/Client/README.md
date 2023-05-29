# Client Library

## About
Client is a simple application built using socket programming in C++ which is capable of receiving files over TCP. 

## Design
The Client communication is achieved by including `#include "getData.h"` file and calling 
`getDataFile("192.xxx.x.x", yy, "foo.txt")` whereas 192.xxx.x.x is IP of server, yy is the port 
of server and "foo.txt" is required file with data by user.            

## Major Files:
`getData.cpp`:
The main function API `getDataFile(serverHostname,server port,fileDataName)` that can be called by users to 
get data from a specific node.

The peer receiver or the server thread, resposible to accept and provide files to other peers.

`PeerClient.cpp`
The peer reciever resposible to connects to other peers servers and give commands to receive specific file from specific server.


## Compilation
Go to the src/Client/build folder and run
`cmake ..`
`make all` to compile
`make clean` to clean




