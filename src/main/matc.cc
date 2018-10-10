#include "../project.cc"

int main(int argc, char *argv[]) {
    for (auto i = 0; i < argc; ({
        std::string arg = argv[i];
        if (arg.size() > 2 && arg.substr(0, 2) == "--" && arg[2] != '#') project::helper::sendSocket(project::helper::command_port, arg.substr(2));
        i++;
    }));
    return 0;
};