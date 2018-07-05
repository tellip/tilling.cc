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
        sprintf(fmt, ":%%%d%c", (int) sizeof(in_port_t) * 8, std::is_signed<in_port_t>::value ? 'd' : 'u');
        in_port_t i;
        if (!sscanf(s, fmt, &i)) error("environment variable \"DISPLAY\"");
        return config::socket_port_base + (in_port_t) (2 * i);
    }();
    const in_port_t event_socket_port = socket_port + (in_port_t) 1;

    const auto sendSocket = [&](const in_port_t &port, const std::string &msg) {
        auto sock_server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock_server < 0) wm::error("socket");
        else {
            sockaddr_in sai_server;
            bzero((char *) &sai_server, sizeof(sai_server));
            sai_server.sin_family = AF_INET;
            sai_server.sin_addr.s_addr = inet_addr(config::socket_host);
            sai_server.sin_port = htons(port);
            if (connect(sock_server, (sockaddr *) &sai_server, sizeof(sai_server)) < 0) wm::error("connect");
            else if (write(sock_server, msg.c_str(), msg.size()) < 0) wm::error("write");
            close(sock_server);
        }
    };

    const auto createSocket = [&](const in_port_t &port, sockaddr_in &sai) {
        auto sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) error("socket");
        int enable = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) error("setsockopt");

        bzero((char *) &sai, sizeof(sai));
        sai.sin_family = AF_INET;
        sai.sin_addr.s_addr = inet_addr(config::socket_host);
        sai.sin_port = htons(port);
        if (bind(sock, (sockaddr *) &sai, sizeof(sai)) < 0) error("bind");
        if (listen(sock, SOMAXCONN) < 0) error("listen");
        return sock;
    };

    const auto acceptSocket = [&](const int &sock, char *const &buffer, const size_t buffer_size) {
        sockaddr_in sai;
        socklen_t len = sizeof(sai);
        auto sock_client = accept(sock, (sockaddr *) &sai, &len);
        if (sock_client < 0) error("accept");

        bzero(buffer, buffer_size);
        if (read(sock_client, buffer, buffer_size) < 0) error("read");

        close(sock_client);
    };
}