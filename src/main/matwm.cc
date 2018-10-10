#include "../project.cc"

int main() {
    {
        std::stringstream ss;
        ss << "if [ -f " << project::config::config_directory << "/bashrc ]; then\n"
           << "\tbash " << project::config::config_directory << "/bashrc\n"
           << "fi";
        system(ss.str().c_str());
    };

    project::server::server([&](const auto &x_connection, const auto &x_default_screen, const auto &break_, const auto &loop, const auto &clean) {
        project::matrix::matrix(x_connection, x_default_screen, break_, [&](const auto &command_handlers, const auto &root_event_mask, const auto &leaf_event_mask, const auto &event_handlers) {
            loop(command_handlers, root_event_mask, leaf_event_mask, event_handlers, [&](const auto &join) {
                join();
                clean();
                {
                    std::stringstream ss;
                    ss << "if [ -f " << project::config::config_directory << "/bash_logout ]; then\n"
                       << "\tbash " << project::config::config_directory << "/bash_logout\n"
                       << "fi";
                    system(ss.str().c_str());
                };
            });
        });
    });

    return EXIT_SUCCESS;
}