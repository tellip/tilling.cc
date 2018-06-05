#include <X11/Xlib.h>
#include "matrix.hh"

namespace wm {
    namespace matrix {
        PointerCoordinate::PointerCoordinate(Display *const &display) : _display(display) {
            _x = _y = -1;
        }

        void PointerCoordinate::refresh() {
            XQueryPointer(_display, XDefaultRootWindow(_display), &_root, &_child, &_root_x, &_root_y, &_win_x, &_win_y, &_mask);
        }

        void PointerCoordinate::record() {
            refresh();
            _x = _root_x;
            _y = _root_y;
        }

        bool PointerCoordinate::check() {
            refresh();
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
                _mask_layer(XCreateSimpleWindow(_display, XDefaultRootWindow(_display), 0, 0, 1, 1, 0, _normal_pixel, _normal_pixel)),
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
                                                leaf->_configure({_display_hv, -(int) config::border_width, -(int) config::border_width, _display_width + config::border_width * 2, _display_height + config::border_width * 2});
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
                                                _activate(leaf);
                                                leaf->_parent->_refresh();
                                                _active->_focus(false);
                                            }
                                            for (auto j = fl.cbegin(); j != fl.cend(); ({
                                                (*j)();
                                                j++;
                                            }));

                                            leaf->_focus(true);
                                            _active = leaf;
                                            _pointer_coordinate.record();
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
                                                wa.x != leaf->_attribute.x ||
                                                wa.y != leaf->_attribute.y ||
                                                wa.width != leaf->_attribute.width - config::border_width * 2 ||
                                                wa.height != leaf->_attribute.height - config::border_width * 2
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
                                        if (_active != leaf && !_pointer_coordinate.check()) {
                                            for (Node *i = leaf; i->_parent; ({
                                                i->_parent->_iter_active = i->_parent_iter;
                                                i = i->_parent;
                                            }));

                                            _active->_focus(false);
                                            leaf->_focus(true);
                                            _active = leaf;
                                        }
                                    }
                                }}
                        }
                ),
                _pointer_coordinate(display) {
            if (_xia_protocols == None || _xia_delete_window == None) error("XInternAtom");
            XMapWindow(_display, _mask_layer);

            _root = _view = _active = nullptr;

            refresh();
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
                if (parent->_iter_active == node->_parent_iter) parent->_iter_active = parent->_children.end();
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
                    sibling->_configure(Attribute(parent->_attribute));
                    delete parent;
                    return sibling;
                } else if (parent->_children.size() > 1) {
                    parent->_configureChildren();
                    return parent;
                } else error("\"parent->_children.size()<1\"");
            }
            return nullptr;
        }

        void Space::_activate(Node *const &node) {
            for (auto i = node; i->_parent && i->_parent->_iter_active != i->_parent_iter; ({
                i->_parent->_iter_active = i->_parent_iter;
                i = i->_parent;
            }));
        }

        void Space::refresh() {
            static bool called = false;

            _display_width = (unsigned int) XDisplayWidth(_display, DefaultScreen(_display));
            _display_height = (unsigned int) XDisplayHeight(_display, DefaultScreen(_display));
            XResizeWindow(_display, _mask_layer, _display_width, _display_height);
            if (!called) {
                called = true;
                _display_hv = (HV) (_display_width >= _display_height);
            }
            if (_view) {
                _view->_configure({_display_hv, -(int) config::border_width, -(int) config::border_width, _display_width + config::border_width * 2, _display_height + config::border_width * 2});
                _view->_refresh();
            }
        }

        void Space::exit() {
            _breakLoop();
        }

        void Space::focus(const HV &hv, const FB &fb) {
            Node *sibling = nullptr;
            for (Node *node = _active; node != _view; ({
                auto parent = node->_parent;
                if (parent->_attribute.hv == hv) {
                    auto i = std::next(node->_parent_iter, fb ? 1 : -1);
                    if (i == parent->_children.end()) {
                        auto grand_parent = parent->_parent;
                        if (parent == _view || grand_parent == _view) {
                            i = std::next(i, fb ? 1 : -1);
                            sibling = *i;

                            node = _view;
                        } else node = grand_parent;
                    } else {
                        sibling = *i;

                        node = _view;
                    }
                } else node = parent;
            }));

            if (sibling) {
                _activate(sibling);
                auto leaf = sibling->_activeLeaf(FB(!fb));
                _active->_focus(false);
                leaf->_focus(true);
                _active = leaf;
            }
        }

        void Space::move(const HV &hv, const FB &fb) {
            if (_active && _active != _view && _active->_parent->_attribute.hv == hv) {
                _move(_active, std::next(_active->_parent_iter, 0.5 + 1.5 * (fb ? 1 : -1)));
                _activate(_active);
                _active->_parent->_refresh();
                _pointer_coordinate.record();
            }
        }

        void Space::join(const HV &hv, const FB &fb) {
            if (_active && _active != _view && _active->_parent->_attribute.hv == hv) {
                std::list<std::function<void()>> fl;
                if (_active->_parent->_children.size() == 2) {
                    fl.emplace_back([&]() {
                        _active->_parent->_refresh();
                    });
                } else {
                    fl.emplace_back([&]() {
                        _active->_parent->_parent->_refresh();
                    });
                }
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
                auto sibling = *i;
                _quit(_active);
                _join(_active, sibling, FB(!fb));
                _activate(_active);

                for (auto j = fl.cbegin(); j != fl.cend(); ({
                    (*j)();
                    j++;
                }));

                _pointer_coordinate.record();
            }
        }

        void Space::quit(const HV &hv, const FB &fb) {
            if (_active && _active != _view && _active->_parent->_attribute.hv != hv) {
                if (_view == _active->_parent) {
                    std::list<std::function<void()>> fl;

                    auto attribute = Attribute(_view->_attribute);
                    if (_root == _view) {
                        fl.emplace_back([&]() {
                            _root = _view;
                        });

                        auto rest = _quit(_active);
                        _join(rest, _active, FB(!fb));
                    } else {
                        auto rest = _quit(_active);
                        _activate(rest);
                        _join(_active, rest->_parent, fb);
                        _quit(rest);
                        _join(rest, _active, FB(!fb));
                    }
                    _active->_parent->_configure(std::move(attribute));
                    _view = _active->_parent;

                    for (auto j = fl.cbegin(); j != fl.cend(); ({
                        (*j)();
                        j++;
                    }));
                } else {
                    auto rest = _quit(_active);
                    _activate(rest);
                    _join(_active, rest->_parent, fb);
                }
                _activate(_active);
                _active->_parent->_refresh();
                _pointer_coordinate.record();
            }
        }

        void Space::review(const FB &fb) {
            auto new_view = (fb ? _view->_activeChild() : _view->_parent);
            if (new_view) {
                new_view->_configure(Attribute(_view->_attribute));
                XRaiseWindow(_display, _mask_layer);
                new_view->_refresh();
                _view = new_view;
                _pointer_coordinate.record();
            }
        }

        void Space::transpose() {
            _display_hv = HV(!_display_hv);
            _view->_refresh();
        }

        Node::Node(Space *const &space) : _space(space) {
            _parent = nullptr;
        }

        Node::~Node() = default;

        void Node::_configure(Attribute &&attribute) {
            _attribute = std::move(attribute);
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
                XMoveResizeWindow(_space->_display, _window, _attribute.x, _attribute.y, _attribute.width - config::border_width * 2, _attribute.height - config::border_width * 2);
                XRaiseWindow(_space->_display, _window);
            }

            Leaf *Leaf::_activeLeaf() {
                return this;
            }

            Leaf *Leaf::_activeLeaf(const FB &) {
                return this;
            }

            Node *Leaf::_activeChild() {
                return this;
            }

            Branch *Leaf::_receive(Node *const &node, const FB &fb) {
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
                new_parent->_configure(Attribute(_attribute));
                return new_parent;
            }

            void Leaf::_focus(const bool &yes) {
                if (yes) {
                    XSetWindowBorder(_space->_display, _window, _space->_focus_pixel);
                    XSetInputFocus(_space->_display, _window, RevertToNone, CurrentTime);
                } else XSetWindowBorder(_space->_display, _window, _space->_normal_pixel);
            }

            Branch::Branch(Space *const &space) : Node(space) {}

            void Branch::_configure(Attribute &&attribute) {
                Node::_configure(std::move(attribute));
                _configureChildren();
            }

            void Branch::_refresh() {
                for (auto i = _children.cbegin(); i != _children.cend(); (*i++)->_refresh());
            }

            Leaf *Branch::_activeLeaf() {
                return (*_iter_active)->_activeLeaf();
            }

            Leaf *Branch::_activeLeaf(const FB &fb) {
                if (_iter_active == _children.end()) _iter_active = (fb ? std::prev(_children.end()) : _children.begin());
                return (*_iter_active)->_activeLeaf(fb);
            }

            Node *Branch::_activeChild() {
                return *_iter_active;
            }

            Branch *Branch::_receive(Node *const &node, const FB &fb) {
                node->_parent_iter = _children.insert(_iter_active == _children.end() ? (fb ? _children.end() : _children.begin()) : (fb ? std::next(_iter_active) : _iter_active), node);
                node->_parent = this;
                _configureChildren();
                return this;
            }

            void Branch::_configureChildren() {
                switch (_attribute.hv) {
                    case HV::HORIZONTAL: {
                        auto s = (int) (_attribute.x + config::border_width);
                        auto d = (unsigned int) ((_attribute.width - config::border_width * 2) / _children.size());
                        for (auto i = _children.cbegin(); i != std::prev(_children.cend()); ({
                            (*i)->_configure({HV(!_attribute.hv), s, _attribute.y + (int) config::border_width, d, _attribute.height - config::border_width * 2});
                            s += d;
                            i++;
                        }));
                        (*std::prev(_children.cend()))->_configure(
                                {
                                        HV(!_attribute.hv),
                                        s, _attribute.y + (int) config::border_width,
                                        _attribute.x + _attribute.width - config::border_width - s, _attribute.height - config::border_width * 2
                                }
                        );
                        break;
                    }
                    case HV::VERTICAL: {
                        auto s = (int) (_attribute.y + config::border_width);
                        auto d = (unsigned int) ((_attribute.height - config::border_width * 2) / _children.size());
                        for (auto i = _children.cbegin(); i != std::prev(_children.cend()); ({
                            (*i)->_configure({HV(!_attribute.hv), _attribute.x + (int) config::border_width, s, _attribute.width - config::border_width * 2, d});
                            s += d;
                            i++;
                        }));
                        (*std::prev(_children.cend()))->_configure(
                                {
                                        HV(!_attribute.hv),
                                        _attribute.x + (int) config::border_width, s,
                                        _attribute.width - config::border_width * 2, _attribute.y + _attribute.height - config::border_width - s
                                }
                        );
                    }
                }
            }
        }
    }
}