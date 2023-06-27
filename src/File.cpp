#include "File.h"
#include "Log.h"
#include "Poco/File.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Path.h"

namespace File {

std::string Locate(std::string file) {
  for (auto mount : Node::getLocalhostNode().mounts) {
    std::string local_file;
    local_file =
        Node::getLocalhostNode().normalizeMountPath(file + "@" + mount.first);
    Poco::File lf(local_file);
    bool found = lf.exists();

    Log::Info("File", "Looking for '%s' at '%s' found: %b", file, lf.path(),
              found);

    if (found)
      return local_file;
  }
  return "";
}
} // namespace File
