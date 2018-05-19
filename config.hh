#pragma once

#include <netinet/in.h>

namespace matrix_wm {
	namespace config {
		const char *const socket_host = "127.0.0.1";
		const in_port_t socket_port_base = 2056;

		const unsigned int border_width = 40;
		const char *const normal_color = "Black", *const focused_color = "#466BB0";
	}
}