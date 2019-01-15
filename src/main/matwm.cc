#include "../project.cc"

int main() {
    project::server::server([&](const auto &x_connection, const auto &x_default_screen, const auto &break_, const auto &loop, const auto &clean) {
        project::matrix::matrix(x_connection, x_default_screen, break_, [&](const auto &command_handlers, const auto &root_event_mask, const auto &leaf_event_mask, const auto &event_handlers) {
            loop(command_handlers, root_event_mask, leaf_event_mask, event_handlers, [&](const auto &join) {
                join();
                clean();
            });
        });
    });

    return EXIT_SUCCESS;
}