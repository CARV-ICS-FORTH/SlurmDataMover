#include "NodeServer.h"
#include "Node.h"
#include "NodeOps.h"
#include "Utils.h"
#include <iostream>

using namespace Poco::Net;

NodeServerHandler ::NodeServerHandler(const Poco::Net::StreamSocket &s)
    : Poco::Net::TCPServerConnection(s) {}

void NodeServerHandler ::run() {
  StreamSocket &sock = socket();
  try {
    const Node &node = Node::getFromIp(sock.peerAddress().host().toString());
    std::string aka = (node) ? ("(aka " + node.name + ")") : "(unknown)";
    std::string op;
    op = sdm_unpack(sock);

    Log::Info("Node", "%s Request from %s%s (%d)", op,
              sock.peerAddress().toString(), aka, sock.available());

    node_op_map.at(op)(sock, aka);

  } catch (Poco::Exception &exc) {
    Log::Error("Node", "Error: %s", exc.displayText());
  } catch (...) {
    Log::Error("Node", "Error: Unknown");
  }
}
