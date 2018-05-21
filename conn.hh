#pragma once

#include <X11/Xlib.h>
#include "main.hh"

namespace matrix_wm {
	auto conn = [&](const auto &callback) {
		auto display = XOpenDisplay(NULL);
		if (display == NULL) error("XOpenDisplay");
		auto display_closed = false;

		bool looping = false;
		try {
			const auto &display_substitute = display;

			callback(
					display_substitute,
					//loopEvents
					[&](long event_masks, const EventHandlers &handlers, const auto &callback) {
						if (!looping) {
							event_masks |= SubstructureNotifyMask;
							XSelectInput(display, XDefaultRootWindow(display), event_masks);
							auto thread_loop = std::thread([&]() {
								looping = true;
								while (looping) {
									XEvent event;
									if (XCheckMaskEvent(display, event_masks, &event)) {
										auto i = handlers.find(event.type);
										if (i != handlers.end()) i->second(event);
									}
								}
//								while (looping) {
//									XEvent event;
//									if (!XNextEvent(display, &event)) error("XNextEvent");
//									auto i = handlers.find(event.type);
//									if (i != handlers.end()) i->second(event);
//								}
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
					},
					[&]() {
						if (looping) looping = false;
//						XEvent event;
//						XSendEvent(display, XDefaultRootWindow(display), False, SubstructureNotifyMask, &event);
					}
			);
		} catch (...) {
			if (!display_closed) XCloseDisplay(display);
			throw true;
		}
	};
}