#include "main.hh"

int main(int argc, char *argv[]) {
	for (auto i = 0; i < argc; i++) {
		auto arg = argv[i];
		if (strlen(arg) > 2 && arg[0] == '-' && arg[1] == '-' && arg[3] != '\n') matrix_wm::sendSock(std::string("c-") + &arg[2]);
	}
	return 0;
};