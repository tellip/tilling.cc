#pragma once

#include <X11/Xlib.h>
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
			typedef std::list<Node *> _Children;
			typedef typename _Children::iterator _Position;
			_Position _position;
			int _x, _y;
			unsigned int _width, _height;
		public:
//			void

			virtual void configure(const HV &, const int &x, const int &y, const unsigned int &width, const unsigned int &height) {
				_x = x;
				_y = y;
				_width = width;
				_height = height;
			}

			virtual void refresh(Display *const &)=0;

			class Leaf;

			class Branch;
		};

		class Node::Leaf : public Node {
		public:
			typedef std::unordered_map<Window, typename Node::Leaf *> Leaves;
			const Window window;
			const typename Leaves::iterator key;

			virtual void refresh(Display *const &display) {
				XMoveResizeWindow(display, window, _x, _y, _width, _height);
			}

			Leaf(const Window &w, const typename Leaves::iterator &k) :
					window(w),
					key(key) {}
		};

		class Node::Branch : public Node {
			Node::_Children _children;
		public:
			void refresh(Display *const &display) {
				for (auto i = _children.cbegin(); i != _children.cend(); i++) {
					(*i)->refresh(display);
				}
			}
		};

		Node *root = NULL, *view = NULL;
		typename Node::Leaf *focused = NULL;
		typename Node::Leaf::Leaves leaves;

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
									auto xmap = event.xmap;
									auto window = xmap.window;
									if (leaves.find(window) == leaves.end() && !xmap.override_redirect) {
										auto i = leaves.insert(std::make_pair(window, (typename Node::Leaf *) NULL)).first;
										i->second = new typename Node::Leaf(window, i);

									}
								}}
						}
				)
		);
	};
}