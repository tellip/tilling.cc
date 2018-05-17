#pragma once

#include "main.hh"

namespace matrix_wm {
	auto conn = [&](Display *&display, std::function<void()> &breakLoop) {
		display = XOpenDisplay(NULL);
		if (display == NULL) error("XOpenDisplay");
		XSelectInput(display, XDefaultRootWindow(display), SubstructureNotifyMask);

		//eventLoop
		return [&](const EventHandlers &handlers) {
			auto looping = true;
			auto thread_loop = std::thread([&]() {
				while (looping) {
					XEvent event;
					if (XCheckMaskEvent(display, SubstructureNotifyMask, &event)) {
						auto i = handlers.find(event.type);
						if (i != handlers.end()) i->second(event);
					}
				}
			});

			breakLoop = [&]() {
				looping = false;

				//clean
				return [&]() {
					XCloseDisplay(display);
				};
			};

			//joinThread
			return [&]() {
				thread_loop.join();
			};
		};
	};
}