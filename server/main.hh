#pragma once

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int, char *[]);

namespace matrix_wm {
    class Config;
}

#include "config.hc"