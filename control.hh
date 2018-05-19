#pragma once

#include <X11/Xlib.h>
#include "main.hh"

namespace matrix_wm {
	auto control = [&](const std::function<void()> &breakListen, Display *const &display, const std::function<void()> &breakLoop, const auto &callback) {
		unsigned long normal_pixel, focused_pixel;
		[&](const auto &cp) {
			normal_pixel = cp(config::normal_color);
			focused_pixel = cp(config::focused_color);
		}([&](const char *const &cc) {
			auto cm = DefaultColormap(display, XDefaultScreen(display));
			XColor x_color;
			XAllocNamedColor(display, cm, cc, &x_color, &x_color);
			return x_color.pixel;
		});

		auto xia_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
		auto xia_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
		if (xia_protocols == None || xia_delete_window == None) error("XInternAtom");

		int display_width, display_height;
		enum HV {
			HORIZONTAL = true, VERTICAL = false
		} display_hv;

		struct Node {
			Node *parent;
			typename std::list<Node *>::iterator position;
			HV hv;
			int x, y;
			unsigned int width, height;

			enum Type {
				Leaf, Branch
			};
			const Type node_type;

			struct Leaf {
				const Window window;
				const typename std::unordered_map<Window, Leaf *>::iterator key;

				Leaf(const Window &w, const typeof(key) &k) : window(w), key(k) {}
			};

			struct Branch {
				std::list<Node *> children;
			};

			union Derived {
				typename Node::Leaf *leaf;
				typename Node::Branch *branch;
			};
			const Derived derived;

			Node(const Type &t, Derived &&d) : node_type(t), derived(std::move(d)) {}

			void poly(const std::unordered_map<int, std::function<void()>> &vf) {
				vf.find(node_type)->second();
			};
		};

//		auto configureNode = [&](Node *const &node, const HV &hv, const int &x, const int &y, const unsigned int &width, const unsigned int &height) {
//			node->hv = hv;
//			node->x = x;
//			node->y = y;
//			node->width = width;
//			node->height = height;
//			node->poly({
//							   {Node::Type::Leaf,   [&]() {}},
//							   {Node::Type::Branch, [&]() {}}
//					   });
//		};
//
//		auto refreshNode = [&](Node *const &node) {
//			node->poly({
//							   {Node::Type::Leaf,   [&]() {
//								   auto nleaf = dynamic_cast<NLeaf *>(node);
//								   XMoveResizeWindow(display, nleaf->window, node->x, node->y, node->width, node->height);
//							   }},
//							   {Node::Type::Branch, [&]() {}}
//					   });
//		};

//		class Node {
//		protected:
//			Display *const _display;
//			Node *_parent;
//
//			_Position _position;
//			int _x, _y;
//			unsigned int _width, _height;
//		public:
////			void
//
//			virtual void configure(const HV &, const int &x, const int &y, const unsigned int &width, const unsigned int &height) {
//				_x = x;
//				_y = y;
//				_width = width;
//				_height = height;
//			}
//
//			virtual void refresh()=0;
//
//			void refreshNormalize() const {
//
//			}
//
//			Node(Display *const &display) :
//					_display(display) {}
//
//			class Leaf;
//
//			class Branch;
//		};
//
//		class Node::Leaf : public Node {
//		public:
//			typedef std::unordered_map<Window, typename Node::Leaf *> Leaves;
//			const Window window;
//			const typename Leaves::iterator key;
//
//			virtual void refresh() {
//				XMoveResizeWindow(_display, window, _x, _y, _width, _height);
//			}
//
//			Leaf(Display *const &d, const Window &w, const typename Leaves::iterator &k) :
//					Node(d),
//					window(w),
//					key(k) {
//				XSetWindowBorderWidth(_display, window, config::border_width);
//			}
//		};
//
//		class Node::Branch : public Node {
//			Node::_Children _children;
//		public:
//			void refresh() {
//				for (auto i = _children.cbegin(); i != _children.cend(); i++) {
//					(*i)->refresh();
//				}
//			}
//
//			Branch(Display *const &display) :
//					Node(display) {}
//		};

//		Node *root = NULL, *view = NULL;
//		typename Node::Leaf *focused = NULL;
//		typename Node::Leaf::Leaves leaves;

		auto refresh = [&]() {
			display_width = XDisplayWidth(display, DefaultScreen(display));
			display_height = XDisplayHeight(display, DefaultScreen(display));
			display_hv = (HV) (display_width >= display_height);
//			if (view) {
//				view->configure(display_hv, 0, 0, display_width, display_height);
//				view->refresh();
//			}
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
//									if (leaves.find(window) == leaves.end() && !xmap.override_redirect) {
//										auto i = leaves.insert(std::make_pair(window, (typename Node::Leaf *) NULL)).first;
////										i->second = new typename Node::Leaf(display, window, i);
//
//									}
								}}
						}
				)
		);
	};
}