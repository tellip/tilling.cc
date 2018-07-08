#pragma once

#include <xcb/xcb.h>
#include "main.hh"

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

        auto server = [&](const auto &callback) {
            sockaddr_in sai_command/*, sai_command_helper, sai_event_helper*/;
            auto sock_command = createSocket(command_port, sai_command)/*, sock_command_helper = createSocket(command_helper_port, sai_command_helper), sock_event_helper = createSocket(event_helper_port, sai_event_helper)*/;

            xcb_connection_t *x_connection;
            xcb_screen_t *x_default_screen;
            xcb_window_t x_default_root_window;
            {
                auto screen_of_display = [&](xcb_connection_t *c, int screen) -> xcb_screen_t * {
                    xcb_screen_iterator_t iter;

                    iter = xcb_setup_roots_iterator(xcb_get_setup(c));
                    for (; iter.rem; --screen, xcb_screen_next(&iter))
                        if (screen == 0)
                            return iter.data;

                    return nullptr;
                };
                int screen_default_nbr;
                x_connection = xcb_connect("Matrix Window Manager", &screen_default_nbr);
                if (xcb_connection_has_error(x_connection)) error("xcb_connect");
                x_default_screen = screen_of_display(x_connection, screen_default_nbr);
                if (x_default_screen) x_default_root_window = x_default_screen->root;
                else error("screen_of_display");
            }

            ({
                auto cookie = xcb_query_tree(x_connection, x_default_root_window);
                xcb_generic_error_t *err = nullptr;
                auto replay = xcb_query_tree_reply(x_connection, cookie, &err);
                if (err) error("xcb_query_tree_reply");
                auto length = xcb_query_tree_children_length(replay);
                if (length) error("child windows already existing");
            });

            bool looping = false;
            callback(
                    x_connection,
                    //break
                    [&]() {
                        if (looping) {
                            looping = false;
                            sendSocket(command_port, "");
                        }
                    },
                    //loop
                    [&](const CommandHandlers &command_handlers, const long &root_event_mask, const long &leaf_event_mask, const EventHandlers &event_handlers, const auto &callback) {
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

                            uint32_t mask[1] = {root_event_mask | leaf_event_mask};
                            xcb_change_window_attributes(x_connection, x_default_root_window, XCB_CW_EVENT_MASK, mask);
                            xcb_flush(x_connection);

                            auto thread_x = std::thread([&]() {
                                while (looping) {
                                    auto event = xcb_wait_for_event(x_connection);
                                    do {
                                        if (event) {
                                            auto type = event->response_type & ~0x80;
                                            if (type) {
                                                auto i = event_handlers.find(type);
                                                if (i != event_handlers.end()) i->second(event);
                                            }
                                            free(event);
                                        }
                                    } while ((event = xcb_poll_for_event(x_connection)));
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
                        xcb_disconnect(x_connection);
                    }
            );
        };
    }
}
