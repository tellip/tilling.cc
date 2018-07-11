#pragma once

#include <netinet/in.h>
#include <vector>

namespace wm {
    namespace config {
        const char *const socket_host = "127.0.0.1";
        const in_port_t socket_port_base = 2056;

        const uint16_t border_width = 1;
        const std::vector<uint16_t> normal_color = {0, 0, 0}, focus_color = {0, 255, 0};
    }
}