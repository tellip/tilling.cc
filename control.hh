#pragma once

#include "main.hh"

namespace matrix_wm {
	

	auto control = [&](
			Display *const &display, const std::function<void()> &breakListen, const std::function<void()> &breakLoop,
			EventHandlers &event_handlers, CommandHandlers &command_handlers
	) {
		auto xia_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
		auto xia_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
		if (xia_protocols == None || xia_delete_window == None) error("XInternAtom");


	};
}