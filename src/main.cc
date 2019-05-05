#include "wm.h"
#include "tree.cc"

#include "regular.cc/regular.hh"

int main(int argc, char **argv) {
    std::string
            arg = argv[0],
            file = ({
        using namespace regular::shortcut::narrow;
        split(arg, plc_om(ps("/\\"))).back();
    });

    std::unordered_map<std::string, std::function<void()>> actions = {
            {"help",   []() {}},
            {"server", []() {
                project::server::server([&](const auto &x_connection, const auto &x_default_screen, const auto &break_, const auto &loop, const auto &clean) {
                    project::tree::tree(x_connection, x_default_screen, break_, [&](const auto &command_handlers, const auto &root_event_mask, const auto &leaf_event_mask, const auto &event_handlers) {
                        loop(command_handlers, root_event_mask, leaf_event_mask, event_handlers, [&](const auto &join) {
                            join();
                            clean();
                        });
                    });
                });
            }},
            {"client", [&]() {
                for (auto i = 2; i < argc; ({
                    std::string arg = argv[i];
                    if (arg.size() > 2 && arg.substr(0, 2) == "--" && arg[2] != '#') project::helper::sendSocket(project::helper::command_port, arg.substr(2));
                    i++;
                }));
            }}
    };

    if (argc < 2 || ({
        auto i = actions.find(argv[1]);
        i == actions.cend() ? true : ({
            i->second();
            false;
        });
    }))
        std::cout << "Arguments are invalid, try \"" + file + " help\".";


    return EXIT_SUCCESS;
}