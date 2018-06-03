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
                                {MapNotify,       [&](const XEvent &event) {
                                    if (!event.xmap.override_redirect) {
                                        auto i = _leaves.find(event.xmap.window);
                                        if (i == _leaves.end()) {
                                            auto leaf = new node::Leaf(this, event.xmap.window);

                                            std::list<std::function<void()>> fl;
                                            if (!_active) {
                                                fl.emplace_back([&]() {
                                                    _root = _view = leaf;
                                                });
                                                leaf->_configure(_display_hv, -config::border_width, -config::border_width, _display_width + config::border_width * 2, _display_height + config::border_width * 2);
                                                leaf->_refresh();
                                            } else {
                                                if (_active == _view) {
                                                    fl.emplace_back([&]() {
                                                        _view = leaf->_parent;
                                                    });
                                                    if (_root == _view) {
                                                        fl.emplace_back([&]() {
                                                            _root = _view;
                                                        });
                                                    }
                                                    _join(leaf, _active, FB::FORWARD);
                                                } else _join(leaf, _active->_parent, FB::FORWARD);
                                                leaf->_parent->_refresh();
                                            }
                                            for (auto j = fl.cbegin(); j != fl.cend(); ({
                                                (*j)();
                                                j++;
                                            }));

                                            _focus(leaf, false);
                                            _pointer_coordinate._record();
                                        } else error("\"_leaves.find(event.xmap.window) != _leaves.end()\"");
                                    }
                                }},
                                {ConfigureNotify, [&](const XEvent &event) {
                                    auto i = _leaves.find(event.xconfigure.window);
                                    if (i != _leaves.end()) {
                                        auto leaf = i->second;
                                        XWindowAttributes wa;
                                        XGetWindowAttributes(_display, leaf->_window, &wa);
                                        if ((
                                                wa.x != leaf->_x ||
                                                wa.y != leaf->_y ||
                                                wa.width != leaf->_width - config::border_width * 2 ||
                                                wa.height != leaf->_height - config::border_width * 2
                                        )) {
                                            leaf->_refresh();
                                        }
                                    }
                                }},
                                {FocusIn,         [&](const XEvent &event) {
                                    auto i = _leaves.find(event.xfocus.window);
                                    if (i != _leaves.end()) {
                                        auto leaf = i->second;
                                        if (leaf != _active) XSetInputFocus(_display, _active->_window, RevertToNone, CurrentTime);
                                    }
                                }},
                                {EnterNotify,     [&](const XEvent &event) {
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

        node::Branch *Space::_join(Node *const &node, Node *const &target, const FB &fb) {
            if (!node->_parent) return target->_receive(node, fb);
            else return nullptr;
        }

        void Space::_move(Node *const &node, const std::list<Node *, std::allocator<Node *>>::iterator &position) {
            if (node->_parent) {
                node->_parent->_children.erase(node->_parent_iter);
                node->_parent_iter = node->_parent->_children.insert(position, node);
                node->_parent->_configureChildren();
            }
        }

        Node *Space::_quit(Node *const &node) {
            auto parent = node->_parent;
            if (parent) {
                parent->_children.erase(node->_parent_iter);
                node->_parent = nullptr;
                if (parent->_children.size() == 1) {
                    auto sibling = parent->_children.front();
                    parent->_children.erase(sibling->_parent_iter);
                    auto grand_parent = parent->_parent;
                    if (grand_parent) {
                        sibling->_parent_iter = grand_parent->_children.insert(parent->_parent_iter, sibling);
                        grand_parent->_children.erase(parent->_parent_iter);
                    }
                    sibling->_parent = grand_parent;
                    sibling->_configure(parent->_hv, parent->_x, parent->_y, parent->_width, parent->_height);
                    delete parent;
                    return sibling;
                } else if (parent->_children.size() > 1) {
                    parent->_configureChildren();
                    return parent;
                } else error("\"parent->_children.size()<1\"");
            }
            return nullptr;
        }

        void Space::_focus(Node *const &node, const bool &from_root) {
            if (from_root) {
                for (auto i = node; i->_parent; ({
                    i->_parent->_iter_active = i->_parent_iter;
                    i = i->_parent;
                }));
            } else {
                for (auto i = node; i->_parent && i->_parent->_iter_active != i->_parent_iter; ({
                    i->_parent->_iter_active = i->_parent_iter;
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

        void Space::focus(const HV &hv, const FB &fb) {
            if (_active && _active != _view) {
                if (_active->_parent->_hv == hv) {
                    auto i = std::next(_active->_parent_iter, fb ? 1 : -1);
                    if (i == _active->_parent->_children.end()) i = std::next(i, fb ? 1 : -1);
                    _focus(*i, false);
                } else if (_active->_parent != _view) {
                    auto i = std::next(_active->_parent->_parent_iter, fb ? 1 : -1);
                    if (i == _active->_parent->_parent->_children.end()) i = std::next(i, fb ? 1 : -1);
                    _focus(*i, false);
                }
            }
        }

        void Space::move(const HV &hv, const FB &fb) {
            if (_active && _active != _view && _active->_parent->_hv == hv) {
                _move(_active, std::next(_active->_parent_iter, 0.5 + 1.5 * (fb ? 1 : -1)));
                _active->_parent->_refresh();
                _focus(_active, false);
                _pointer_coordinate._record();
            }
        }

        void Space::join(const HV &hv, const FB &fb) {
            if (_active && _active != _view && _active->_parent->_hv == hv) {
                std::list<std::function<void()>> fl;
                if (_active->_parent == _view && _active->_parent->_children.size() == 2) {
                    fl.emplace_back([&]() {
                        _view = _active->_parent;
                    });
                    if (_root == _view) {
                        fl.emplace_back([&]() {
                            _root = _view;
                        });
                    }
                }

                auto i = std::next(_active->_parent_iter, fb ? 1 : -1);
                if (i == _active->_parent->_children.end()) i = std::next(i, fb ? 1 : -1);
                auto node = _quit(_active);
                auto parent = _join(_active, *i, FB(!fb));
                node->_refresh();
                parent->_refresh();

                for (auto j = fl.cbegin(); j != fl.cend(); ({
                    (*j)();
                    j++;
                }));

                _focus(_active, false);
                _pointer_coordinate._record();
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
                    _leaves_iter(space->_leaves.insert(std::make_pair(window, this)).first) {
                XSetWindowBorderWidth(space->_display, window, config::border_width);
                XSelectInput(space->_display, window, leaf_event_mask);
            }

            Leaf::~Leaf() {
                _space->_leaves.erase(_leaves_iter);
            }

            void Leaf::_refresh() {
                XMoveResizeWindow(_space->_display, _window, _x, _y, _width - config::border_width * 2, _height - config::border_width * 2);
            }

            node::Leaf *Leaf::_getActiveLeaf() {
                return this;
            }

            node::Branch *node::Leaf::_receive(Node *const &node, const FB &fb) {
                auto new_parent = new Branch(_space);
                if (_parent) {
                    new_parent->_parent_iter = _parent->_children.insert(_parent_iter, new_parent);
                    _parent->_children.erase(_parent_iter);
                }
                new_parent->_parent = _parent;
                _parent_iter = new_parent->_children.insert(new_parent->_children.begin(), this);
                _parent = new_parent;
                node->_parent_iter = new_parent->_children.insert(fb ? new_parent->_children.end() : new_parent->_children.begin(), node);
                node->_parent = new_parent;
                new_parent->_configure(_hv, _x, _y, _width, _height);
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
                return (*_iter_active)->_getActiveLeaf();
            }

            node::Branch *Branch::_receive(Node *const &node, const FB &fb) {
                node->_parent_iter = _children.insert(fb ? std::next(_iter_active) : _iter_active, node);
                node->_parent = this;
                _configureChildren();
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