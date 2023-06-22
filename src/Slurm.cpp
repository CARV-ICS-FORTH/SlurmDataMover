#include "Slurm.h"

#include "Poco/Environment.h"

namespace SLURM {
bool inSLURM() { return Poco::Environment::has("SLURM_JOB_NODELIST"); }

std::string getHeadNode() {
  std::string head;
  if (inSLURM()) {
    std::string node_env = Poco::Environment::get("SLURM_JOB_NODELIST");
    node_env = node_env.substr(0, node_env.find_first_of(","));
    bool after_cage = false;
    for (auto c : node_env)
      switch (c) {
      case '[':
        after_cage = true;
        break;
      case '-':
        if (after_cage)
          return head;
      default:
        head += c;
      }
  }
  return head;
}
} // namespace SLURM
