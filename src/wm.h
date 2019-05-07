#pragma once

#include "regular.cc/regular.hh"
#include "json.cc/json.hh"

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
        using EventHandler=std::function<void(xcb_generic_event_t *const &)>;
        using EventHandlers=std::unordered_map<
                int,
                EventHandler
        >;
        using CommandHandler=std::function<void()>;
        using CommandHandlers=std::unordered_map<
                std::string,
                CommandHandler
        >;
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
    }
}

#include "tree.h"
#include "config.h"
#include "helper.h"
#include "server.hh"