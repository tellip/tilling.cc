#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include "public.hh"

int main(int argc, char *argv[]) {
	for (auto i = 0; i < argc; i++) {

		std::string arg = argv[i];
		if (arg.size() >= 2 && arg.substr(0, 2) == "--") wm::sendSock(arg.substr(2));
	}
	return 0;
};
