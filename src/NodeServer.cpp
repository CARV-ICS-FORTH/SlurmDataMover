#include "NodeServer.h"
#include "Node.h"
#include "NodeOps.h"
#include "Poco/Util/Application.h"
#include "Utils.h"
#include <iostream>

using namespace Poco::Net;
using namespace Poco::Util;

NodeServerHandler ::NodeServerHandler(const Poco::Net::StreamSocket &s)
    : Poco::Net::TCPServerConnection(s) {}

void NodeServerHandler ::run() {
  StreamSocket &sock = socket();
  Application &app = Application::instance();
  Poco::Logger &log = app.logger();
  try {
    const Node &node = Node::getFromIp(sock.peerAddress().host().toString());
    std::string aka = (node) ? ("(aka " + node.name + ")") : "(unknown)";
    std::string op;
    op = sdm_unpack(sock);

    log.information("[Node ] %s Request from %s%s (%d)", op,
                    sock.peerAddress().toString(), aka, sock.available());

    node_op_map.at(op)(sock, aka, log);

  } catch (Poco::Exception &exc) {
    log.error("[Node ] Error: %s", exc.displayText());
  } catch (...) {
    log.error("[Node ] Error: Unknown");
  }
}
