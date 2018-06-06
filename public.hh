#pragma once

#include "config.hh"

namespace wm {
    const auto error = [&](const char *const &fn) {
        std::cout << "ERROR on " << fn << '\n';
        throw true;
    };

    const in_port_t socket_port = [&]() {
        auto s = getenv("DISPLAY");
        char fmt[99];
        sprintf(fmt, ":%%%d%c", sizeof(in_port_t) * 8, std::is_signed<in_port_t>::value ? 'd' : 'u');
        in_port_t i;
        if (!sscanf(s, fmt, &i)) error("environment variable \"DISPLAY\"");
        return config::socket_port_base + i;
    }();

    const auto sendSock = [&](const std::string &msg) {
        auto sock_server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock_server < 0) wm::error("socket");
        else {
            sockaddr_in sai_server;
            bzero((char *) &sai_server, sizeof(sai_server));
            sai_server.sin_family = AF_INET;
            sai_server.sin_addr.s_addr = inet_addr(config::socket_host);
            sai_server.sin_port = htons(socket_port);
            if (connect(sock_server, (sockaddr *) &sai_server, sizeof(sai_server)) < 0) wm::error("connect");
            else if (write(sock_server, msg.c_str(), msg.size()) < 0) wm::error("write");
            close(sock_server);
        }
    };
}