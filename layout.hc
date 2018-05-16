#pragma once

#include "helper.hc"

namespace matrix_wm {
	class Layout {
		virtual ~Layout() = 0;

	public:
		enum HV {
			HORIZONTAL, VERTICAL
		};

	private:
		class _Node {
		protected:
			virtual ~_Node() = 0;

			_Node *_parent;
			std::list<_Node *>::iterator *_position;
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

		class _NMeta : public _Node {
			Window _window;
		public:
			inline void refresh() {
				XMoveWindow(display, _window, _x, _y);
				XResizeWindow(display, _window, _width, _height);
			}
		};

		class _NComp : public _Node {
			std::list<_Node *> _children;
		public:
			inline void refresh() {
				for (auto i = _children.cbegin(); i != _children.cend(); i++) {
					(*i)->refresh();
				}
			}
		};

		static Atom _xia_protocols, _xia_delete_window;

		static int _display_width, _display_height;
		static HV _display_hv;
		static _Node *_root, *_view;
		static _NMeta *_focused;
		static std::unordered_map<Window, _NMeta *> _metas;

	public:
		static void initialize() {
			static bool called = false;
			if (!called) {
				called = true;
				_xia_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
				_xia_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
				if (_xia_protocols == None || _xia_delete_window == None) error("XInternAtom");
				_root = _view = _focused = NULL;
				refresh();
			}
		}

		static void refresh() {
			_display_width = XDisplayWidth(display, DefaultScreen(display));
			_display_height = XDisplayHeight(display, DefaultScreen(display));
			_display_hv = (HV) (_display_width < _display_height);
			if (_view) {
				_view->configure(_display_hv, 0, 0, _display_width, _display_height);
				_view->refresh();
			}
		}

		static const std::unordered_map<std::string, std::function<void()>> commands;
		static const std::unordered_map<int, std::function<void()>> event_handlers;
	};

	typeof(Layout::_xia_protocols) Layout::_xia_protocols;
	typeof(Layout::_xia_delete_window) Layout::_xia_delete_window;
	typeof(Layout::_display_width) Layout::_display_width;
	typeof(Layout::_display_height) Layout::_display_height;
	typeof(Layout::_display_hv) Layout::_display_hv;
	typeof(Layout::_root) Layout::_root;
	typeof(Layout::_view) Layout::_view;
	typeof(Layout::_focused) Layout::_focused;


	typeof(Layout::commands) Layout::commands = {
			{"exit", [&]() {
				looping = false;
			}}
	};

	typeof(Layout::event_handlers) Layout::event_handlers = {
			{MapNotify, [&]() {

			}}
	};

}