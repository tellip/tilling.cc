#include "main.hh"

int main() {
	matrix_wm::sock([&](const auto &listenCommands, const auto &breakListen) {
		matrix_wm::conn([&](const auto &display, const auto &loopEvents, const auto &breakLoop) {
			matrix_wm::control(breakListen, display, breakLoop, [&](const auto &command_handlers, const auto &root_event_masks, const auto &leaf_event_masks, const auto &event_handlers) {
				listenCommands(breakLoop, command_handlers, [&](const auto &joinListen) {
					loopEvents(root_event_masks, leaf_event_masks, event_handlers, [&](const auto &joinLoop) {
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