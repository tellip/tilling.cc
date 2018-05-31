#include <X11/Xlib.h>
#include "matrix.hh"

namespace wm {
	namespace matrix {

		unsigned long Space::_colorPixel(const char *const &cc) {
			auto cm = DefaultColormap(_display, XDefaultScreen(_display));
			XColor x_color;
			if (XAllocNamedColor(_display, cm, cc, &x_color, &x_color) == 0) error("XAllocNamedColor");
			return x_color.pixel;
		}

		Space::Space(Display *const &display) :
				_display(display),
				_normal_pixel(_colorPixel(config::normal_color)),
				_focus_pixel(_colorPixel(config::normal_color)),
				_xia_protocols(XInternAtom(display, "WM_PROTOCOLS", False)),
				_xia_delete_window(XInternAtom(display, "WM_DELETE_WINDOW", False)),
				event_handlers(
						{
								{MapNotify, [&](const XEvent &event, const auto &done) {
									if (!event.xmap.override_redirect) {
										auto i = _leaves.find(event.xmap.window);
										if (i == _leaves.end()) {
											auto leaf = new node::Leaf(this, event.xmap.window);
											if (!_focus) {
												std::cout << _display_height << '\n';
												leaf->configure(_display_hv, -config::border_width, -config::border_width, _display_width + config::border_width * 2, _display_height + config::border_width * 2);
												leaf->refresh();
												_root = _view = leaf;
											} else if (_focus == _view) {

											}
											focus(leaf, true);
										} else error("\"_leaves.find(event.xmap.window) != _leaves.end()\"");
									}
									done();
								}}
						}
				) {
			if (_xia_protocols == None || _xia_delete_window == None) error("XInternAtom");

			_display_width = (unsigned int) XDisplayWidth(display, XDefaultScreen(display));
			_display_height = (unsigned int) XDisplayHeight(display, XDefaultScreen(display));
			_display_hv = HV(_display_width > _display_height);

			_root = _view = _focus = NULL;
		}

		void Space::refresh() {
			_display_width = (unsigned int) XDisplayWidth(_display, DefaultScreen(_display));
			_display_height = (unsigned int) XDisplayHeight(_display, DefaultScreen(_display));
			_display_hv = (HV) (_display_width >= _display_height);
			if (_view) {
				_view->configure(_display_hv, -config::border_width, -config::border_width, _display_width + config::border_width * 2, _display_height + config::border_width * 2);
				_view->refresh();
			}
		}

		void Space::focus(Node *const &node, const bool &to_set) {
			for (auto i = node; i->_parent && i->_parent->_active_iter != i->_iter_parent; ({
				i->_parent->_active_iter = i->_iter_parent;
				i = i->_parent;
			}));
			auto active = node->getActiveLeaf();
			if (_focus != active) {
				if (_focus) XSetWindowBorder(_display, active->_window, _normal_pixel);
				XSetWindowBorder(_display, active->_window, _focus_pixel);
				if (to_set) XSetInputFocus(_display, active->_window, RevertToNone, CurrentTime);
				_focus = active;
			}
		}

		Node::Node(Space *const &space) : _space(space) {}

		Node::~Node() {}

		void Node::configure(const HV &hv, const int &x, const int &y, const unsigned int &width, const unsigned int &height) {
			_hv = hv;
			_x = x;
			_y = y;
			_width = width;
			_height = height;
		}

		namespace node {
			Leaf::Leaf(Space *const &space, const Window &window) :
					Node(space),
					_window(window),
					_iter_leaves(space->_leaves.insert(std::make_pair(window, this)).first) {
				XMapWindow(space->_display, window);
				XSetWindowBorderWidth(space->_display, window, config::border_width);
				XSelectInput(space->_display, window, leaf_event_mask);
			}

			Leaf::~Leaf() {
				_space->_leaves.erase(_iter_leaves);
			}

			void Leaf::refresh() {
				XMoveResizeWindow(_space->_display, _window, _x, _y, _width - config::border_width * 2, _height - config::border_width * 2);
			}

			node::Leaf *Leaf::getActiveLeaf() {
				return this;
			}

			Branch::Branch(Space *const &space) : Node(space) {}

			void Branch::configure(const HV &hv, const int &x, const int &y, const unsigned int &width, const unsigned int &height) {
				Node::configure(hv, x, y, width, height);
				configureChildren();
			}

			void Branch::refresh() {
				for (auto i = _children.cbegin(); i != _children.cend(); (*i++)->refresh());
			}

			node::Leaf *Branch::getActiveLeaf() {
				return (*_active_iter)->getActiveLeaf();
			}

			void Branch::configureChildren() {
				switch (_hv) {
					case HV::HORIZONTAL: {
						auto s = _x + config::border_width;
						auto d = (_width - config::border_width * 2) / _children.size();
						for (auto i = _children.cbegin(); i != std::prev(_children.cend()); i++) {
							(*i)->configure(HV(!_hv), s, _y + config::border_width, d, _height - config::border_width * 2);
							s += d;
						}
						(*std::prev(_children.cend()))->configure(HV(!_hv), s, _y + config::border_width, _x + _width - config::border_width - s, _height - config::border_width * 2);
						break;
					}
				}
			}
		}
	}
}