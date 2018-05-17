#include "main.hh"

int main() {
	auto sockListen = matrix_wm::sock();
	Display *display;
	auto eventLoop = matrix_wm::conn(display);

	return 0;
}