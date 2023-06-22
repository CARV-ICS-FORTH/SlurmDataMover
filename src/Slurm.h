#pragma once

#include <string>

namespace SLURM {
/**
 * Return true if we are running inside SLURM.
 *
 * \note Currently we use the SLURM_JOB_NODELIST env-var to detect SLURM.
 */
bool inSLURM();
/**
 * Return Head Node if running in SLURM, emtpy string otherwise
 */
std::string getHeadNode();
} // namespace SLURM
