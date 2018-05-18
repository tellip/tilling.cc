#include "main.hh"

int main() {
	matrix_wm::sock([&](const auto &breakListen, const auto &listenCommands) {
		matrix_wm::conn([&](const auto &display, const auto &breakLoop, const auto &loopEvents) {
			matrix_wm::control(breakListen, display, breakLoop, [&](const auto &command_handlers, const auto &event_handlers) {
				listenCommands(command_handlers, [&](const auto &joinListen) {
					loopEvents(event_handlers, [&](const auto &joinLoop) {
						joinListen([&](const auto &cleanSock) {
							joinLoop([&](const auto &cleanConn) {
								cleanSock();
								cleanConn();
							});
						});
					});
				});
			});
		});
	});

	return 0;
}