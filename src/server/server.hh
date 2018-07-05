#pragma once

#include "main.hh"

namespace wm {
    namespace server {
        using EventHandler=std::function<void(const XEvent &)>;
        using EventHandlers=std::unordered_map<
                int,
                EventHandler
        >;
        using CommandHandlers=std::unordered_map<
                std::string,
                std::function<void()>
        >;

        auto server = [&](const auto &callback) {
            sockaddr_in sai_command, sai_event;
            auto sock_command = createSocket(command_port, sai_command), sock_event = createSocket(event_helper_port, sai_event);

            const auto display = XOpenDisplay(nullptr);
            if (display == nullptr) error("XOpenDisplay");

            ({
                Window root, parent, *children;
                unsigned int children_size;
                XQueryTree(display, XDefaultRootWindow(display), &root, &parent, &children, &children_size);
                if (children_size) error("child windows already existing");
            });

            bool looping = false;
            callback(
                    display,
                    //break
                    [&]() {
                        if (looping) {
                            looping = false;
                            sendSocket(command_port, "");
                        }
                    },
                    //loop
                    [&](const CommandHandlers &command_handlers, const long &root_event_mask, const long &leaf_event_mask, const EventHandlers &event_handlers, const auto &callback) {
                        XEvent event;
                        EventHandler event_handler;
                        if (!looping) {
                            looping = true;

                            auto thread_sock = std::thread([&]() {
                                while (looping) {
                                    char buffer[256];
                                    acceptSocket(sock_command, buffer, 256);

                                    if (!strcmp(buffer, "#event")) {
                                        event_handler(event);
                                        sendSocket(event_helper_port, "#done");
                                    } else {
                                        auto i = command_handlers.find(buffer);
                                        if (i != command_handlers.end()) i->second();
                                    }
                                }
                            });

                            XSelectInput(display, XDefaultRootWindow(display), root_event_mask);
                            auto thread_x = std::thread([&]() {
                                while (looping) {
                                    XNextEvent(display, &event);
                                    auto i = event_handlers.find(event.type);
                                    if (i != event_handlers.end()) {
                                        event_handler = i->second;
                                        sendSocket(command_port, "#event");
                                        char buffer[256];
                                        acceptSocket(sock_event, buffer, 256);
                                    }
                                }
                            });

                            callback(
                                    //join
                                    [&]() {
                                        thread_sock.join();
                                        thread_x.join();
                                    }
                            );
                        }
                    },
                    //clean
                    [&]() {
                        close(sock_command);
                        close(sock_event);
                        XCloseDisplay(display);
                    }
            );
        };
    }
}