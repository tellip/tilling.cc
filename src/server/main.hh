#pragma once

#include <arpa/inet.h>
#include <cmath>
#include <cstring>
#include <functional>
#include <iostream>
#include <list>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <X11/Xlib.h>

#include "../helpers.hh"

namespace wm {
    namespace matrix {
        enum HV {
            HORIZONTAL = true, VERTICAL = false
        };

        enum FB {
            FORWARD = true, BACKWARD = false
        };

        class PointerCoordinate;

        class Space;

        struct Attribute;

        class Node;

        namespace node {
            class Branch;

            class Leaf;
        }
    }
}

#include "server.hh"
#include "matrix.hh"