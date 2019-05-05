#pragma once

#include "wm.h"

namespace project::config {
    const auto directory = "TREE_WM_DIRECTORY", config_file = "TREE_WM_CONFIG_FILE";

    const auto socket_host = "127.0.0.1";
    const in_port_t socket_port_base = 2056;

    const uint16_t border_width = 1;
    const std::string normal_color = "black", focused_color = "green";
}