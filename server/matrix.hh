#pragma once

#include "main.hh"

namespace wm {
	namespace matrix {
		enum HV {
			HORIZONTAL = true, VERTICAL = false
		};

		enum FB {
			FORWARD = true, BACKWARD = false
		};

		class Space {
		public:
			Display *const display;
			const unsigned long normal_pixel, focus_pixel;
			const Atom xia_protocols, xia_delete_window;

		private:
			unsigned int _display_width, _display_height;
			HV _display_hv;

			Node *_root, *_view;
			node::Leaf *_active;
			std::unordered_map<Window, Node *> _leaves;

		public:

			Space(Display *const &);

		private:
			static unsigned long _colorPixel(Display *const &, const char *const &);
		};

		class Node {
		protected:
			Space *_space;

			Node *_parent;
			std::list<Node *>::iterator _position;
			HV _hv;
			int _x, _y;
			unsigned int _width, _height;

		public:
			virtual ~Node() = 0;
		};

		namespace node {
			class Branch : public Node {

			};

			class Leaf : public Node {

			};
		}

		auto main = [&](Display *const &display, const auto &callback) {
			CommandHandlers command_handlers;

			callback(
					command_handlers,
					SubstructureNotifyMask,
					NoEventMask,
					EventHandlers(
							{
									{MapNotify, [&](const XEvent &event) {

									}}
							}
					),
					[&](const auto &break_, const std::string &handling_event_command_name, const auto &handleEvent) {
						command_handlers["exit"] = break_;
						command_handlers[handling_event_command_name] = handleEvent;
					});
		};
	}
}