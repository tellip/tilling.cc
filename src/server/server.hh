#pragma once

#include "main.hh"

namespace wm {
    namespace server {
        using EventHandler=std::function<void(const XEvent &)>;
        using EventHandlers=std::unordered_map<
                int,
                EventHandler
        >;
        using CommandHandler=std::function<void()>;
        using CommandHandlers=std::unordered_map<
                std::string,
                CommandHandler
        >;

        auto server = [&](const auto &callback) {
            sockaddr_in sai_command/*, sai_command_helper, sai_event_helper*/;
            auto sock_command = createSocket(command_port, sai_command)/*, sock_command_helper = createSocket(command_helper_port, sai_command_helper), sock_event_helper = createSocket(event_helper_port, sai_event_helper)*/;

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
                        std::queue<CommandHandler> command_tasks;
                        /*bool handling_command = false, command_waiting = false, handling_event = false, event_waiting = false;*/
                        if (!looping) {
                            looping = true;

                            auto thread_sock = std::thread([&]() {
                                while (looping) {
                                    char buffer[256];
                                    acceptSocket(sock_command, buffer, 256);
                                    auto i = command_handlers.find(buffer);
                                    if (i != command_handlers.end()) command_tasks.push(i->second);
                                }
                            });

                            XSelectInput(display, XDefaultRootWindow(display), root_event_mask);
                            auto thread_x = std::thread([&]() {
                                while (looping) {
                                    do {
                                        XNextEvent(display, &event);
                                        auto i = event_handlers.find(event.type);
                                        if (i != event_handlers.end()) i->second(event);
                                    } while (XPending(display) > 0);
                                    while (!command_tasks.empty()) {
                                        command_tasks.front()();
                                        command_tasks.pop();
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
                        /*close(sock_command_helper);
                        close(sock_event_helper);*/
                        XCloseDisplay(display);
                    }
            );
        };
    }
}
