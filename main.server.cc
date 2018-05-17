#include "main.hh"

int main() {
	std::function<void()> breakListen;
	auto sockListen = matrix_wm::sock(breakListen);

	Display *display;
	std::function<void()> breakLoop;
	auto eventLoop = matrix_wm::conn(display, breakLoop);

	matrix_wm::EventHandlers event_handlers;
	matrix_wm::CommandHandlers command_handlers;
	matrix_wm::control(
			display, breakListen, breakLoop,
			event_handlers, command_handlers
	);

	auto joinLoop = eventLoop(event_handlers);
	auto joinListen = sockListen(command_handlers);

	joinLoop();
	joinListen();

	return 0;
}