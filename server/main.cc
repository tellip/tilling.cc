#include "main.hh"

int main() {
    wm::server::server([&](const auto &display, const auto &loop, const auto &clean) {
        wm::matrix::matrix(display, [&](const auto &command_handlers, const auto &root_event_mask, const auto &leaf_event_mask, const auto &event_handlers, const auto &handling_event_command_name, const auto &callback) {
            loop(command_handlers, root_event_mask, leaf_event_mask, event_handlers, handling_event_command_name, [&](const auto &break_, const auto &handleEvent, const auto &join) {
                callback(break_, handleEvent);
                join();
                clean();
            });
        });
    });

    return EXIT_SUCCESS;
}