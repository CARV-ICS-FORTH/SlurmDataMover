# Server Library

## About
Server is a simple application built using socket programming in C++ which is capable of sending files over TCP to clients. 

## Design
The Server communication is achieved by including `#include "startServer.h"` file and calling 
`StartTheServer("192.xxx.x.x", yy)` whereas 192.xxx.x.x is IP of server, yy is the port 
of server required to start a server on a node to wait for requests for clients.            

## Major Files:
`startServer.cpp`:
The main function API `StartTheServer(serverHostname,server port)` that can be called by users to 
send data to a specifics nodes.

The peer receiver or the server thread, resposible to accept and provide files to other peers.

`PeerServer.cpp`
The peer server resposible to accepts connects to various client nodes and sends specific files to that clients .


## Compilation
Go to the src/server/build folder and run
`cmake ..`
`make all` to compile
`make clean` to clean
