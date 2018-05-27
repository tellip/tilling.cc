#include "main.hh"

int main() {
	matrix_wm::sock([&](const auto &listenCommands, const auto &breakListen) {
		matrix_wm::conn([&](const auto &display, const auto &loopEvents, const auto &breakLoop) {
			auto break_ = [&]() {
				breakListen();
				breakLoop();
			};
			matrix_wm::control(display, break_, [&](const auto &command_handlers, const auto &root_event_masks, const auto &leaf_event_masks, const auto &event_handlers) {
				try {
					listenCommands(command_handlers, [&](const auto &joinListen) {
						loopEvents(root_event_masks, leaf_event_masks, event_handlers, [&](const auto &joinLoop) {
							joinListen([&](const auto &cleanSock) {
								joinLoop([&](const auto &cleanConn) {
									cleanSock();
									cleanConn();
								});
							});
						});
					});
				} catch (...) {
					break_();
					throw true;
				}
			});
		});
	});

	return 0;
}