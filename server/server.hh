#pragma once

#include "main.hh"

namespace wm {
    namespace server {
        typedef std::unordered_map<
                int,
                std::function<void(const XEvent &)>
        > EventHandlers;
        typedef std::unordered_map<
                std::string,
                std::function<void()>
        > CommandHandlers;

        auto server = [&](const auto &callback) {
            auto sock_server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
            if (sock_server < 0) error("socket");
            int enable = 1;
            if (setsockopt(sock_server, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) error("setsockopt");

            sockaddr_in sai_server;
            bzero((char *) &sai_server, sizeof(sai_server));
            sai_server.sin_family = AF_INET;
            sai_server.sin_addr.s_addr = inet_addr(config::socket_host);
            sai_server.sin_port = htons(socket_port);
            if (bind(sock_server, (sockaddr *) &sai_server, sizeof(sai_server)) < 0) error("bind");
            if (listen(sock_server, SOMAXCONN) < 0) error("listen");

            const auto display = XOpenDisplay(nullptr);
            if (display == nullptr) error("XOpenDisplay");

            callback(
                    display,
                    //loop
                    [&](const CommandHandlers &command_handlers, const long &root_event_mask, const long &leaf_event_mask, const EventHandlers &event_handlers, const std::string &handling_event_command_name, const auto &callback) {
                        bool looping = true;
                        bool handling_event;
                        bool handling_command = false;

                        auto thread_sock = std::thread([&]() {
                            while (looping) {
                                sockaddr_in sai_client;
                                socklen_t len = sizeof(sai_client);
                                auto sock_client = accept(sock_server, (sockaddr *) &sai_client, &len);
                                if (sock_client < 0) error("accept");

                                if (!handling_event) handling_command = true;

                                char buffer[256];
                                bzero(buffer, sizeof(buffer));
                                if (read(sock_client, buffer, sizeof(buffer)) < 0) error("read");

                                auto i = command_handlers.find(buffer);
                                if (i != command_handlers.end()) i->second();
                                else std::cout << buffer << '\n';

                                handling_command = false;

                                close(sock_client);
                            }
                        });

                        XSelectInput(display, XDefaultRootWindow(display), root_event_mask);
                        XEvent event;
                        auto thread_x = std::thread([&]() {
                            while (looping) {
                                while (handling_command);
                                if (XCheckMaskEvent(display, root_event_mask | leaf_event_mask, &event)) {
                                    handling_event = true;
                                    sendSock(handling_event_command_name);
                                    while (handling_event);
                                }
                            }
                        });

                        callback(
                                //break
                                [&]() {
                                    if (looping) {
                                        looping = false;
                                        sendSock("");
                                    }
                                },
                                //handleEvent
                                [&]() {
                                    auto i = event_handlers.find(event.type);
                                    if (i != event_handlers.end()) i->second(event);
                                    handling_event = false;
                                },
                                //join
                                [&]() {
                                    thread_sock.join();
                                    thread_x.join();
                                }
                        );
                    },
                    //clean
                    [&]() {
                        close(sock_server);
                        XCloseDisplay(display);
                    }
            );
        };
    }
}