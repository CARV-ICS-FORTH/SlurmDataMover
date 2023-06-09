#include "File.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"

using Poco::JSON::Array;
using Poco::JSON::Object;
using Poco::JSON::Parser;

File ::File(const std::string &file_name) : file_name(file_name), size(0) {
  nodes.insert(Node::getHostname());
}
void File ::toHTML(std::ostream &os) const {
  os << "<tr><td>" << file_name << "</td>\n";
  os << "<td>" << location << "</td>\n";
  os << "<td>" << size << "</td>\n";
  os << "<td>";
  for (auto node : nodes) {
    os << " " << node;
  }
  os << "</td></tr>\n";
}
std::string File ::getKey() const { return "file:" + file_name; }
std::string File ::toJSON() const {
  Object obj;
  Array nodes;

  obj.set("name", file_name);

  for (auto node : this->nodes)
    nodes.add(node);

  obj.set("nodes", nodes);
  obj.set("size", size);

  std::stringstream ss;
  obj.stringify(ss);
  return ss.str();
}
void File ::fromJSON(const std::string &json) {
  Parser parser;
  Object::Ptr obj = parser.parse(json).extract<Object::Ptr>();

  file_name = obj->getValue<std::string>("name");
  size = obj->getValue<size_t>("size");

  Array::Ptr nodes = obj->getArray("nodes");

  this->nodes.clear();
  for (auto node : (*nodes))
    this->nodes.insert(node.toString());
}
bool operator==(const File &lhs, const File &rhs) {
  return lhs.file_name == rhs.file_name;
}

const File File::NotFound("NotFound");