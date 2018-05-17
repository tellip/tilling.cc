#pragma once

#include "main.hh"

namespace matrix_wm {
	namespace layout {
		enum HV {
			HORIZONTAL, VERTICAL
		};

		class Node {
		protected:
			virtual ~Node() = 0;

			Display *_display;
			Node *_parent;
			std::list<Node *>::iterator *_position;
			int _x, _y, _width, _height;
		public:
			virtual inline void configure(const HV &, const int &x, const int &y, const int &width, const int &height) {
				_x = x;
				_y = y;
				_width = width;
				_height = height;
			}

			virtual void refresh() = 0;
		};

		class NMeta : public Node {
			Window _window;
		public:
			inline void refresh() {
				XMoveWindow(_display, _window, _x, _y);
				XResizeWindow(_display, _window, _width, _height);
			}
		};

		class NComp : public Node {
			std::list<Node *> _children;
		public:
			inline void refresh() {
				for (auto i = _children.cbegin(); i != _children.cend(); i++) {
					(*i)->refresh();
				}
			}
		};

		std::function<void()> refresh;
		auto initialize = [&](Display *const &display) {
			auto xia_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
			auto xia_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
			if (xia_protocols == None || xia_delete_window == None) error("XInternAtom");
			int display_width, display_height;
			HV display_hv;
			Node *root = NULL, *view = NULL;
			NMeta *focused = NULL;
			refresh = [&]() {
				display_width = XDisplayWidth(display, DefaultScreen(display));
				display_height = XDisplayHeight(display, DefaultScreen(display));
				display_hv = (HV) (display_width < display_height);
				if (view) {
					view->configure(display_hv, 0, 0, display_width, display_height);
					view->refresh();
				}
			};
			refresh();
		};
	}
}