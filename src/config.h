#pragma once

#include "wm.h"

namespace wm::config {
    const auto socket_host = "127.0.0.1";
    const in_port_t socket_port_base = 2056;

    const auto config_path_evn = "TREE_WM_CONFIG_PATH";
}