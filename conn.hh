#pragma once

#include "main.hh"

namespace matrix_wm {
	auto conn = [&](Display *&display) {
		display = XOpenDisplay(NULL);
		if (display == NULL) error("XOpenDisplay");
		XSelectInput(display, XDefaultRootWindow(display), SubstructureNotifyMask);

		//eventLoop
		return [&](const std::unordered_map<
				int,
				std::function<void(const XEvent &)>
		> &handlers) {
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

			//breakLoop
			return [&]() {
				looping = false;

				//joinLoopThread
				return [&]() {
					thread_loop.join();

					//clean
					return [&]() {
						XCloseDisplay(display);
					};
				};
			};
		};
	};
}