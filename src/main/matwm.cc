#include "../project.cc"

int main() {
    system("if [ -f $HOME/.config/matwm/bashrc ]; then\n"
           "\tbash $HOME/.config/matwm/bashrc\n"
           "fi");
    project::server::server([&](const auto &x_connection, const auto &x_default_screen, const auto &break_, const auto &loop, const auto &clean) {
        project::matrix::matrix(x_connection,x_default_screen, break_, [&](const auto &command_handlers, const auto &root_event_mask, const auto &leaf_event_mask, const auto &event_handlers) {
            loop(command_handlers, root_event_mask, leaf_event_mask, event_handlers, [&](const auto &join) {
                join();
                clean();
                system("if [ -f $HOME/.config/matwm/bash_logout ]; then\n"
                       "\tbash $HOME/.config/matwm/bash_logout\n"
                       "fi");
            });
        });
    });

    return EXIT_SUCCESS;
}