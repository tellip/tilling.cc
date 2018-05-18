#pragma once

#include "main.hh"

namespace matrix_wm {
	auto conn = [&](const auto &callback) {
		auto display = XOpenDisplay(NULL);
		if (display == NULL) error("XOpenDisplay");
		auto display_closed = false;
		try {
			const auto &display_substitute = display;
			XSelectInput(display_substitute, XDefaultRootWindow(display), SubstructureNotifyMask);

			bool looping;
			callback(
					display,
					//breakLoop
					[&]() {
						looping = false;
					},
					//loopEvents
					[&](const EventHandlers &handlers, const auto &callback) {
						looping = true;
						auto thread_loop = std::thread([&]() {
							while (looping) {
								XEvent event;
								if (XCheckMaskEvent(display, SubstructureNotifyMask, &event)) {
									auto i = handlers.find(event.type);
									if (i != handlers.end()) i->second(event);
								}
							}
						});

						callback(
								//joinThread
								[&](const auto &callback) {
									thread_loop.join();
									callback(
											//clean
											[&]() {
												display_closed = true;
												XCloseDisplay(display);
											}
									);
								}
						);
					}
			);
		} catch (...) {
			if (!display_closed) XCloseDisplay(display);
			throw true;
		}
	};
}