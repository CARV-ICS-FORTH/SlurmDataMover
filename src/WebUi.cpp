#include "WebUi.h"
#include "Poco/Net/HTTPServerResponse.h"
#include <sstream>

#include "File.h"
#include "Node.h"
#include "Redis.h"
#include "Utils.h"

using namespace Poco::Net;

class WebUiHandler : public HTTPRequestHandler {
  template <class T>
  void writeTable(std::ostream &os, const std::unordered_set<T> &data,
                  void (T::*fn)(std::ostream &os) const,
                  std::vector<const char *> headers) {
    if (!data.size())
      return;
    Tag tag(os, "div class='card col-5'");
    {
      Tag table(os, "table class='striped'");
      {
        Tag th(os, "thead class='bg-red-200'");
        Tag tr(os, "tr");
        for (auto &header : headers) {
          Tag th(os, "th", header);
        }
      }
      {
        Tag tbody(os, "tbody class='bg-grey-100'");

        for (const T &d : data)
          (d.*fn)(os);
      }
    }
  }

  void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {
    Log::Info("WebUI", "Request from %s", request.clientAddress().toString());

    response.setChunkedTransferEncoding(true);
    response.setContentType("text/html");

    std::stringstream oss;
    oss << "<!DOCTYPE html>" << std::endl;
    {
      Tag html(oss, "html lang='en'");
      {
        Tag head(oss, "head");
        { Tag title(oss, "title", "SDM Billboard @ " + Node::getHostname()); }
        Tag style(
            oss, "link rel='stylesheet' href='https://unpkg.com/chota@latest'");
      }
      Tag body(oss, "body style='background-color:#ddd'");
      {
        Tag nav(oss, "nav class='nav'");
        Tag div(oss, "div class='nav-center'");
        Tag a(oss, "a class='brand'",
              "SLURM Data Manager @ " + Node::getHostname());
      }
      Tag col1(oss, "div class='row is-marginless'");
      { Tag col1(oss, "div class='col-1'"); }
      {
        writeTable(oss, Redis::getAllNodes(), &Node::toHTML,
                   {"Hostname", "Mounts", "Addresses"});
        writeTable(oss, File::files, &File::toHTML,
                   {"File", "Location", "Size", "Nodes"});
      }
    }
    response.send() << oss.str();
  }
};

HTTPRequestHandler *
WebUiFactory ::createRequestHandler(const HTTPServerRequest &) {
  return new WebUiHandler;
}
