#include "wm.h"
#include "tree.cc"

int main(int argc, char **argv) {
    std::string
            arg = argv[0],
            file = ({
        using namespace reg::shortcut::narrow;
        split(arg, plc_om(ps("/\\"))).back();
    });

    std::unordered_map<std::string, std::function<void()>> actions = {
            {"help",   [&]() {
                std::cout << file << " ...\n"
                          << "\t" << "help\n"
                          << "\t" << "server\n"
                          << "\t" << "client ...\n";
                std::list<std::string> command_list;
                for (auto i = wm::tree::command_handlers.cbegin();
                     i != wm::tree::command_handlers.cend(); command_list.emplace_back((i++)->first));
                command_list.sort();
                for (auto i = command_list.cbegin(); i != command_list.cend(); std::cout << "\t\t" << *(i++) << "\n");
            }},
            {"server", []() {
                wm::server::server([&](const auto &x_connection, const auto &x_default_screen, const auto &break_,
                                       const auto &loop, const auto &clean) {
                    wm::tree::tree(x_connection, x_default_screen, break_,
                                   [&](const auto &command_handler, const auto &root_event_mask,
                                       const auto &leaf_event_mask, const auto &event_handler) {
                                       loop(command_handler, root_event_mask, leaf_event_mask, event_handler,
                                            [&](const auto &join) {
                                                join();
                                            });
                                   });
                    clean();
                });
            }},
            {"client", [&]() {
                for (auto i = 2; i < argc; wm::helper::sendSocket(wm::helper::command_port(), argv[i++]));
            }}
    };

    if (argc < 2 || ({
        auto i = actions.find(argv[1]);
        i == actions.cend() ? true : ({
            i->second();
            false;
        });
    }))
        std::cout << "Arguments are invalid, try \"" + file + " help\".\n";


    return EXIT_SUCCESS;
}
