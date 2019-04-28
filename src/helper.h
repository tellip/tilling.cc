#pragma once

#include "project.h"

namespace project::helper {
    const auto error = [](const char *const &fn) {
        std::cout << "ERROR on " << fn << '\n';
        throw std::exception();
    };

    const in_port_t command_port = []() {
        auto s = getenv("DISPLAY");
        char fmt[99];
        sprintf(fmt, ":%%%d%c", (int) sizeof(in_port_t) * 8, std::is_signed<in_port_t>::value ? 'd' : 'u');
        in_port_t i;
        if (!sscanf(s, fmt, &i)) error("environment variable \"DISPLAY\"");
        return config::socket_port_base + i /*(in_port_t) (3 * i)*/;
    }();
    /*const in_port_t command_helper_port = command_port + (in_port_t) 1, event_helper_port = command_port + (in_port_t) 2;*/

    const auto sendSocket = [](const in_port_t &port, const std::string &msg) {
        auto sock_server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock_server < 0) project::helper::error("socket");
        else {
            sockaddr_in sai_server{};
            bzero((char *) &sai_server, sizeof(sai_server));
            sai_server.sin_family = AF_INET;
            sai_server.sin_addr.s_addr = inet_addr(config::socket_host);
            sai_server.sin_port = htons(port);
            if (connect(sock_server, (sockaddr *) &sai_server, sizeof(sai_server)) < 0) project::helper::error("connect");
            else if (write(sock_server, msg.c_str(), msg.size()) < 0) project::helper::error("write");
            close(sock_server);
        }
    };

    const auto createSocket = [](const in_port_t &port, sockaddr_in &sai) {
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

    const auto acceptSocket = [](const int &sock, char *const &buffer, const size_t buffer_size) {
        sockaddr_in sai{};
        socklen_t len = sizeof(sai);
        auto sock_client = accept(sock, (sockaddr *) &sai, &len);
        if (sock_client < 0) error("accept");

        bzero(buffer, buffer_size);
        if (read(sock_client, buffer, buffer_size) < 0) error("read");

        close(sock_client);
    };

    const auto screen_of_display = [](xcb_connection_t *c, int screen) -> xcb_screen_t * {
        xcb_screen_iterator_t iter;

        iter = xcb_setup_roots_iterator(xcb_get_setup(c));
        for (; iter.rem; --screen, xcb_screen_next(&iter))
            if (screen == 0)
                return iter.data;

        return nullptr;
    };
}