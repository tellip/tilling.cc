#pragma once

#include "main.hh"

namespace matrix_wm {
    class Config {
        virtual ~Config() = 0;

    public:
        static const unsigned short border_width = 4;
        static const char *const normal_color, *const focus_color;
    };

    typeof(Config::normal_color) Config::normal_color = "Black";
    typeof(Config::focus_color) Config::focus_color = "#466BB0";
}