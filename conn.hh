#pragma once

#include "main.hh"

namespace matrix_wm {
	auto conn = [&](const auto &callback) {
		auto display = XOpenDisplay(NULL);
		if (display == NULL) error("XOpenDisplay");
		auto display_closed = false;
		try {
			const auto &display_substitute = display;

			bool looping;
			callback(
					display,
					//breakLoop
					[&]() {
						looping = false;
					},
					//loopEvents
					[&](const long &event_masks, const EventHandlers &handlers, const auto &callback) {
						XSelectInput(display_substitute, XDefaultRootWindow(display), event_masks);
						looping = true;
						auto thread_loop = std::thread([&]() {
							while (looping) {
								XEvent event;
								if (XCheckMaskEvent(display, event_masks, &event)) {
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