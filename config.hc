#pragma once

namespace matrix_wm {
    class Config {
        virtual ~Config() = 0;

    public:
        static const char *const localhost;
        static const unsigned int socket_port_base = 2056;

        static const unsigned short border_width = 4;
        static const char *const normal_color, *const focus_color;
    };

    typeof(Config::localhost) Config::localhost = "127.0.0.1";
    typeof(Config::normal_color) Config::normal_color = "Black";
    typeof(Config::focus_color) Config::focus_color = "#466BB0";
}