#include <X11/Xlib.h>
#include "matrix.hh"

namespace wm {
    namespace matrix {
        PointerCoordinate::PointerCoordinate(Display *const &display) : _display(display) {
            _x = _y = -1;
        }

        void PointerCoordinate::_refresh() {
            XQueryPointer(_display, XDefaultRootWindow(_display), &_root, &_child, &_root_x, &_root_y, &_win_x, &_win_y, &_mask);
        }

        void PointerCoordinate::_record() {
            _refresh();
            _x = _root_x;
            _y = _root_y;
        }

        bool PointerCoordinate::_check() {
            _refresh();
            return _x == _root_x && _y == _root_y && ({
                _x = _y = -1;
                true;
            });
        }

        Space::Space(Display *const &display, const std::function<void()> &break_) :
                _breakLoop(break_),
                _display(display),
                _normal_pixel(_colorPixel(config::normal_color)),
                _focus_pixel(_colorPixel(config::focus_color)),
                _xia_protocols(XInternAtom(display, "WM_PROTOCOLS", False)),
                _xia_delete_window(XInternAtom(display, "WM_DELETE_WINDOW", False)),
                event_handlers(
                        {
                                {MapNotify,   [&](const XEvent &event) {
                                    if (!event.xmap.override_redirect) {
                                        auto i = _leaves.find(event.xmap.window);
                                        if (i == _leaves.end()) {
                                            auto leaf = new node::Leaf(this, event.xmap.window);
                                            if (!_active) {
                                                leaf->_configure(_display_hv, -config::border_width, -config::border_width, _display_width + config::border_width * 2, _display_height + config::border_width * 2);
                                                leaf->_refresh();
                                                _root = _view = leaf;
                                            } else if (_active == _view) {
                                                auto parent = _join(leaf, _active, FB::FORWARD);
                                                parent->_configure(_display_hv, -config::border_width, -config::border_width, _display_width + config::border_width * 2, _display_height + config::border_width * 2);
                                                parent->_refresh();
                                                _view = parent;
                                            } else {
                                                auto parent = _join(leaf, _active->_parent, FB::FORWARD);
                                                parent->_configureChildren();
                                                parent->_refresh();
                                            }
                                            _focus(leaf, false);
                                            _pointer_coordinate._record();
                                        } else error("\"_leaves.find(event.xmap.window) != _leaves.end()\"");
                                    }
                                }},
                                {FocusIn,     [&](const XEvent &event) {
                                    auto i = _leaves.find(event.xfocus.window);
                                    if (i != _leaves.end()) {
                                        auto leaf = i->second;
                                        if (leaf != _active) XSetInputFocus(_display, _active->_window, RevertToNone, CurrentTime);
                                    }
                                }},
                                {EnterNotify, [&](const XEvent &event) {
                                    auto i = _leaves.find(event.xcrossing.window);
                                    if (i != _leaves.end()) {
                                        auto leaf = i->second;
                                        if (_active != leaf && !_pointer_coordinate._check()) _focus(leaf, true);
                                    }
                                }}
                        }
                ),
                _pointer_coordinate(display) {
            if (_xia_protocols == None || _xia_delete_window == None) error("XInternAtom");

            _display_width = (unsigned int) XDisplayWidth(display, XDefaultScreen(display));
            _display_height = (unsigned int) XDisplayHeight(display, XDefaultScreen(display));
            _display_hv = HV(_display_width > _display_height);

            _root = _view = _active = nullptr;
        }

        unsigned long Space::_colorPixel(const char *const &cc) {
            auto cm = DefaultColormap(_display, XDefaultScreen(_display));
            XColor x_color;
            if (XAllocNamedColor(_display, cm, cc, &x_color, &x_color) == 0) error("XAllocNamedColor");
            return x_color.pixel;
        }

        node::Branch *Space::_join(wm::matrix::Node *const &node, wm::matrix::Node *const &target, const wm::matrix::FB &fb) {
            if (!node->_parent) return target->_receive(node, fb);
            else return nullptr;
        }

        void Space::_move(wm::matrix::Node *const &node, const std::list<wm::matrix::Node *, std::allocator<wm::matrix::Node *>>::iterator &position) {
            if (node->_parent) {
                node->_parent->_children.erase(node->_iter_parent);
                node->_iter_parent = node->_parent->_children.insert(position, node);
            }
        }

        void Space::_focus(Node *const &node, const bool &from_root) {
            if (from_root) {
                for (auto i = node; i->_parent; ({
                    i->_parent->_active_iter = i->_iter_parent;
                    i = i->_parent;
                }));
            } else {
                for (auto i = node; i->_parent && i->_parent->_active_iter != i->_iter_parent; ({
                    i->_parent->_active_iter = i->_iter_parent;
                    i = i->_parent;
                }));
            }
            auto leaf = node->_getActiveLeaf();
            if (_active != leaf) {
                if (_active) XSetWindowBorder(_display, _active->_window, _normal_pixel);
                XSetWindowBorder(_display, leaf->_window, _focus_pixel);
                _active = leaf;
                XSetInputFocus(_display, leaf->_window, RevertToNone, CurrentTime);
            }
        }

        void Space::refresh(const bool &first_time) {
            _display_width = (unsigned int) XDisplayWidth(_display, DefaultScreen(_display));
            _display_height = (unsigned int) XDisplayHeight(_display, DefaultScreen(_display));
            if (first_time) _display_hv = (HV) (_display_width >= _display_height);
            if (_view) {
                _view->_configure(_display_hv, -config::border_width, -config::border_width, _display_width + config::border_width * 2, _display_height + config::border_width * 2);
                _view->_refresh();
            }
        }

        void Space::exit() {
            _breakLoop();
        }

        void Space::focus(const wm::matrix::HV &hv, const FB &fb) {
            if (_active && _active != _view && _active->_parent->_hv == hv) {
                auto i = std::next(_active->_iter_parent, fb ? 1 : -1);
                if (i == _active->_parent->_children.end()) i = std::next(i, fb ? 1 : -1);
                _focus(*i, false);
            }
        }

        void Space::transpose() {
            _display_hv = HV(!_display_hv);
            refresh();
        }

        Node::Node(Space *const &space) : _space(space) {
            _parent = nullptr;
        }

        Node::~Node() = default;

        void Node::_configure(const HV &hv, const int &x, const int &y, const unsigned int &width, const unsigned int &height) {
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
                XSetWindowBorderWidth(space->_display, window, config::border_width);
                XSelectInput(space->_display, window, leaf_event_mask);
            }

            Leaf::~Leaf() {
                _space->_leaves.erase(_iter_leaves);
            }

            void Leaf::_refresh() {
                XMoveResizeWindow(_space->_display, _window, _x, _y, _width - config::border_width * 2, _height - config::border_width * 2);
            }

            node::Leaf *Leaf::_getActiveLeaf() {
                return this;
            }

            node::Branch *node::Leaf::_receive(wm::matrix::Node *const &node, const wm::matrix::FB &fb) {
                auto new_parent = new Branch(_space);
                if (_parent) {
                    new_parent->_iter_parent = _parent->_children.insert(_iter_parent, new_parent);
                    _parent->_children.erase(_iter_parent);
                }
                new_parent->_parent = _parent;
                new_parent->_children.push_back(this);
                _iter_parent = new_parent->_children.begin();
                _parent = new_parent;
                node->_iter_parent = new_parent->_children.insert(fb ? new_parent->_children.end() : new_parent->_children.begin(), node);
                node->_parent = new_parent;
                return new_parent;
            }

            Branch::Branch(Space *const &space) : Node(space) {}

            void Branch::_configure(const HV &hv, const int &x, const int &y, const unsigned int &width, const unsigned int &height) {
                Node::_configure(hv, x, y, width, height);
                _configureChildren();
            }

            void Branch::_refresh() {
                for (auto i = _children.cbegin(); i != _children.cend(); (*i++)->_refresh());
            }

            node::Leaf *Branch::_getActiveLeaf() {
                return (*_active_iter)->_getActiveLeaf();
            }

            node::Branch *Branch::_receive(wm::matrix::Node *const &node, const wm::matrix::FB &fb) {
                node->_iter_parent = _children.insert(fb ? std::next(_active_iter) : _active_iter, node);
                node->_parent = this;
                return this;
            }

            void Branch::_configureChildren() {
                switch (_hv) {
                    case HV::HORIZONTAL: {
                        auto s = _x + config::border_width;
                        auto d = (_width - config::border_width * 2) / _children.size();
                        for (auto i = _children.cbegin(); i != std::prev(_children.cend()); ({
                            (*i)->_configure(HV(!_hv), s, _y + config::border_width, d, _height - config::border_width * 2);
                            s += d;
                            i++;
                        }));
                        (*std::prev(_children.cend()))->_configure(
                                HV(!_hv),
                                s, _y + config::border_width,
                                _x + _width - config::border_width - s, _height - config::border_width * 2
                        );
                        break;
                    }
                    case HV::VERTICAL: {
                        auto s = _y + config::border_width;
                        auto d = (_height - config::border_width * 2) / _children.size();
                        for (auto i = _children.cbegin(); i != std::prev(_children.cend()); ({
                            (*i)->_configure(HV(!_hv), _x + config::border_width, s, _width - config::border_width * 2, d);
                            s += d;
                            i++;
                        }));
                        (*std::prev(_children.cend()))->_configure(
                                HV(!_hv),
                                _x + config::border_width, s,
                                _width - config::border_width * 2, _y + _height - config::border_width - s
                        );
                    }
                }
            }
        }
    }
}