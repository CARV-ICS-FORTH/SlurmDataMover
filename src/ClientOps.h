#pragma once

#include "File.h"
#include "Node.h"

void put_file(const Node &local_server, uint16_t cmd_port, std::string file);
void get_file(const Node &local_server, uint16_t cmd_port, std::string file);
void update_file(const Node &local_server, std::string remote,
                 uint16_t cmd_port);
