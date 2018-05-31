#pragma once

#include <X11/Xlib.h>
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
			Display *const _display;
			const unsigned long _normal_pixel, _focus_pixel;
			const Atom _xia_protocols, _xia_delete_window;

			unsigned int _display_width, _display_height;
			HV _display_hv;

			Node *_root, *_view;
			node::Leaf *_focus;
			std::unordered_map<Window, node::Leaf *> _leaves;
		public:
			const EventHandlers event_handlers;

			Space(Display *const &);

			void refresh();

		private:
			static unsigned long _colorPixel(Display *const &, const char *const &);

			friend Node;
			friend node::Branch;
			friend node::Leaf;
		};

		class Node {
		protected:
			Space *const _space;

			node::Branch *_parent;

			std::list<Node *>::iterator _iter_parent;
			HV _hv;
			int _x, _y;
			unsigned int _width, _height;

		public:
			inline const typeof(_parent) &parent() const { return _parent; }

			inline const typeof(_iter_parent) &iterParent() const { return _iter_parent; }

			Node(Space *const &);

			virtual ~Node() = 0;

			virtual void configure(const HV &, const int &, const int &, const unsigned int &, const unsigned int &);

			virtual void refresh()=0;

			void focus(const bool &);

		protected:
			void _activate();

		public:
			virtual node::Leaf *getActiveLeaf()=0;
		};

		namespace node {
			class Leaf : public Node {
				const Window _window;
				const std::unordered_map<Window, Leaf *>::iterator _iter_leaves;
			public:
				Leaf(Space *const &, const Window &);

				~Leaf();

				void refresh();

				node::Leaf *getActiveLeaf();

				void refreshFocus(const bool &);
			};

			class Branch : public Node {
				std::list<Node *> _children;
				std::list<Node *>::iterator _active_iter;
			public:
				inline const typeof(_active_iter) &activeIter() const { return _active_iter; };

				Branch(Space *const &);

				void configure(const HV &, const int &, const int &, const unsigned int &, const unsigned int &);

				void refresh();

				node::Leaf *getActiveLeaf();

				void configureChildren();

				void activateChild(Node *const &);
			};
		}

		const long root_event_mask = SubstructureNotifyMask;
		const long leaf_event_mask = FocusChangeMask | EnterWindowMask;

		auto main = [&](Display *const &display, const auto &callback) {
			auto space = new Space(display);
			space->refresh();

			CommandHandlers command_handlers = {};
			callback(
					command_handlers,
					root_event_mask,
					leaf_event_mask,
					space->event_handlers,
					[&](const auto &break_, const std::string &handling_event_command_name, const auto &handleEvent) {
						command_handlers["exit"] = break_;
						command_handlers[handling_event_command_name] = handleEvent;
					}
			);
		};
	}
}