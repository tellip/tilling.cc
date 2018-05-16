#pragma once

#include <iostream>

namespace matrix_wm {
    inline void error(const char *const &fn) {
        std::cout << "ERROR on " << fn << '\n';
        throw true;
    }
}