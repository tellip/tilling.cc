#pragma once

#include "regular.cc/include.hh"
#include "json.cc/include.hh"

#include <arpa/inet.h>
#include <cmath>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <netinet/in.h>
#include <queue>
#include <sstream>
#include <stdint-gcc.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <xcb/xcb.h>

namespace wm {
    namespace server {
        template<typename>
        struct Handler;
    }
    namespace tree {
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

        struct CommandHandler;
        struct EventHandler;
    }
}

#include "config.h"
#include "helper.h"
#include "server.hh"
#include "tree.h"