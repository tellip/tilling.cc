#include "main.hh"

int main() {
    wm::server::server([&](const auto &display, const auto &break_, const auto &loop, const auto &clean) {
        wm::matrix::matrix(display, break_, [&](const auto &command_handlers, const auto &root_event_mask, const auto &leaf_event_mask, const auto &event_handlers) {
            loop(command_handlers, root_event_mask, leaf_event_mask, event_handlers, [&](const auto &join) {
                join();
                system("if [ -f $HOME/.config/matwm/bash_exit ]; then\n"
                       "\tbash $HOME/.config/matwm/bash_exit\n"
                       "fi");
                clean();
            });
        });
    });

    return EXIT_SUCCESS;
}