#pragma once

#include "wm.h"

namespace wm::server {
    template<typename Task>
    struct Handler {
        virtual void handle(const Task &) const = 0;
    };

    const auto server = [](const auto &callback) {
        sockaddr_in sai_command{}/*, sai_command_helper, sai_event_helper*/;
        auto sock_command = helper::createSocket(helper::command_port(), sai_command)/*, sock_command_helper = createSocket(command_helper_port, sai_command_helper), sock_event_helper = createSocket(event_helper_port, sai_event_helper)*/;

        xcb_connection_t *x_connection;
        xcb_screen_t *x_default_screen;
        {
            int screen_default_nbr;
            x_connection = xcb_connect(nullptr, &screen_default_nbr);
            if (xcb_connection_has_error(x_connection)) helper::error("xcb_connect");
            x_default_screen = helper::screen_of_display(x_connection, screen_default_nbr);
            if (!x_default_screen->root) helper::error("screen_of_display");
        }

        if (xcb_query_tree_reply(
                x_connection,
                xcb_query_tree(x_connection, x_default_screen->root),
                nullptr
        )->length)
            helper::error("child windows already existing");

        bool looping = false;
        callback(
                x_connection,
                x_default_screen,
                //break
                [&]() {
                    if (looping) {
                        looping = false;
                        helper::sendSocket(helper::command_port(), "");
                    }
                },
                //loop
                [&](const Handler<std::string> &command_handler, const uint32_t &root_event_mask, const uint32_t &leaf_event_mask, const Handler<xcb_generic_event_t *> &event_handler, const auto &callback) {
                    std::queue<std::string> command_tasks;
                    /*bool handling_command = false, command_waiting = false, handling_event = false, event_waiting = false;*/
                    if (!looping) {
                        looping = true;

                        auto thread_sock = std::thread([&]() {
                            while (looping) {
                                char buffer[256];
                                helper::acceptSocket(sock_command, buffer, 256);
                                command_tasks.push(buffer);

                                auto event = new xcb_generic_event_t();
                                xcb_send_event(x_connection, 0, x_default_screen->root, XCB_EVENT_MASK_NO_EVENT, (const char *) event);
                                xcb_flush(x_connection);
                                delete event;
                            }
                        });

                        xcb_change_window_attributes(x_connection, x_default_screen->root, XCB_CW_EVENT_MASK, ({
                            uint32_t values[] = {root_event_mask};
                            values;
                        }));
                        xcb_flush(x_connection);

                        auto thread_x = std::thread([&]() {
                            while (looping) {
                                auto event = xcb_wait_for_event(x_connection);
                                do {
                                    if (event) {
//                                        auto type = event->response_type & ~0x80;
                                        event_handler.handle(event);
                                        free(event);
                                    }
                                } while ((event = xcb_poll_for_event(x_connection)));
                                while (!command_tasks.empty()) {
                                    command_handler.handle(command_tasks.front());
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
