#pragma once

#include "main.hh"

namespace matrix_wm {
	auto control = [&](const std::function<void()> &breakListen, Display *const &display, const std::function<void()> &breakLoop, const auto &callback) {
		auto xia_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
		auto xia_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
		if (xia_protocols == None || xia_delete_window == None) error("XInternAtom");

		int display_width, display_height;
		enum HV {
			HORIZONTAL = true, VERTICAL = false
		} display_hv;

		class Node {
		protected:
			Node *_parent;
			typename std::list<Node *>::iterator _position;
			int _x, _y, _width, _height;
		public:
			void parent(Node *const &target) {}

			virtual void configure(const HV &, const int &x, const int &y, const int &width, const int &height) {
				_x = x;
				_y = y;
				_width = width;
				_height = height;
			}

			virtual void refresh(Display *const &)=0;

			class Meta;

			class Comp;
		};

		class Node::Meta : public Node {
			Window _window;
		public:
			virtual void refresh(Display *const &display) {
				XMoveWindow(display, _window, _x, _y);
				XResizeWindow(display, _window, _width, _height);
			}
		};

		class Node::Comp : public Node {
			std::list<Node *> _children;
		public:
			void refresh(Display *const &display) {
				for (auto i = _children.cbegin(); i != _children.cend(); i++) {
					(*i)->refresh(display);
				}
			}
		};

		Node *root = NULL, *view = NULL;
		typename Node::Meta *focused = NULL;

		auto refresh = [&]() {
			display_width = XDisplayWidth(display, DefaultScreen(display));
			display_height = XDisplayHeight(display, DefaultScreen(display));
			display_hv = (HV) (display_width >= display_height);
			if (view) {
				view->configure(display_hv, 0, 0, display_width, display_height);
				view->refresh(display);
			}
		};

		refresh();

		callback(
				//commands_handlers
				CommandHandlers(
						{
								{"exit", [&]() {
									breakListen();
									breakLoop();
								}}
						}
				),
				//event_masks
				SubstructureNotifyMask,
				//event_handlers
				EventHandlers(
						{
								{MapNotify, [&](const XEvent &event) {

								}}
						}
				)
		);
	};
}