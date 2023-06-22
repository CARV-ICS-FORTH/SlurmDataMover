#include "WebUi.h"
#include "Poco/Net/HTTPServerResponse.h"
#include <sstream>

#include "File.h"
#include "Log.h"
#include "Node.h"
#include "Redis.h"
#include "Utils.h"

using namespace Poco::Net;

class WebUiHandler : public HTTPRequestHandler {
  template <class T, class Cont>
  void writeTable(std::ostream &os, const Cont &data,
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

  void body(std::ostream &os) {
    {
      Tag nav(os, "nav class='nav'");
      Tag div(os, "div class='nav-center'");
      Tag a(os, "a class='brand'",
            "SLURM Data Manager @ " + Node::getHostname());
    }
    {
      Tag col1(os, "div class='row is-marginless'");
      { Tag col1(os, "div class='col-1'"); }
      {
        writeTable(os, Redis::getAllNodes(), &Node::toHTML,
                   {"Status", "Hostname", "Mounts", "Addresses"});
        writeTable(os, Redis::getAllFiles(), &File::toHTML,
                   {"File", "Location", "Size", "Nodes"});
      }
    }
    {
      Tag col1(os, "div class='row is-marginless'");
      { Tag col1(os, "div class='col-1'"); }
      {
        writeTable(os, Log::getLastEntries(), &Log::Entry::toHTML,
                   {"Host", "Type", "Scope", "Message"});
      }
    }
  }

  void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {
    static int updates = 0;

    response.setChunkedTransferEncoding(true);
    response.setContentType("text/html");

    std::stringstream oss;
    if ("/update" != request.getURI()) {
      updates = 0;
      oss << "<!DOCTYPE html>" << std::endl;
      {
        Tag html(oss, "html lang='en'");
        {
          Tag head(oss, "head");
          { Tag title(oss, "title", "SDM Billboard @ " + Node::getHostname()); }
          Tag style(
              oss,
              "link rel='stylesheet' href='https://unpkg.com/chota@latest'");
        }
        Tag body_tag(oss, "body style='background-color:#ddd'");
        body(oss);
      }
      {
        Tag(oss, "script",
            "function update() {fetch(new Request('/update')).then((response) "
            "=> response.blob()).then((blob) => blob.text()).then( (raw) => "
            "{\nbody = document.getElementsByTagName('body')[0];\n "
            "body.innerHTML=raw;\n})}\nsetInterval(update,200)");
      }
    } else {
      body(oss); // Update only sends body
      updates++;
    }

    if (updates % 100 == 0)
      Log::Info("Http", "Request '%s' from %s (%d updates)", request.getURI(),
                request.clientAddress().toString(), updates);

    response.send() << oss.str();
  }
};

HTTPRequestHandler *
WebUiFactory ::createRequestHandler(const HTTPServerRequest &) {
  return new WebUiHandler;
}
