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
                                            if (!_focus) {
                                                leaf->configure(_display_hv, -config::border_width, -config::border_width, _display_width + config::border_width * 2, _display_height + config::border_width * 2);
                                                leaf->refresh();
                                                _root = _view = leaf;
                                            } else if (_focus == _view) {
                                                auto parent = join(leaf, _focus, FB::FORWARD);
                                                parent->configure(_display_hv, -config::border_width, -config::border_width, _display_width + config::border_width * 2, _display_height + config::border_width * 2);
                                                parent->refresh();
                                                _view = parent;
                                            } else {
                                                auto parent = join(leaf, _focus->_parent, FB::FORWARD);
                                                parent->configureChildren();
                                                parent->refresh();
                                            }
                                        } else error("\"_leaves.find(event.xmap.window) != _leaves.end()\"");
                                    }
                                }},
                                {FocusIn,     [&](const XEvent &event) {
                                    auto i = _leaves.find(event.xfocus.window);
                                    if (i != _leaves.end()) {
                                        auto leaf = i->second;
                                        if (_focus != leaf) {
                                            if (_focus) XSetWindowBorder(_display, _focus->_window, _normal_pixel);
                                            XSetWindowBorder(_display, leaf->_window, _focus_pixel);
                                            _focus = leaf;
                                        }
                                    }
                                }},
                                {EnterNotify, [&](const XEvent &event) {
                                    auto i = _leaves.find(event.xcrossing.window);
                                    if (i != _leaves.end()) {
                                        auto leaf = i->second;
                                        focus(leaf);
                                    }
                                }}
                        }
                ) {
            if (_xia_protocols == None || _xia_delete_window == None) error("XInternAtom");

            _display_width = (unsigned int) XDisplayWidth(display, XDefaultScreen(display));
            _display_height = (unsigned int) XDisplayHeight(display, XDefaultScreen(display));
            _display_hv = HV(_display_width > _display_height);

            _root = _view = _focus = nullptr;

            command_handlers = {
                    {"focus-left", [&]() {
                        if (_focus && _focus != _view && _focus->_parent->_hv == HV::HORIZONTAL) {
                            auto i = std::prev(_focus->_iter_parent);
                            if (i == _focus->_parent->_children.end()) i = std::prev(i);
                            focus(*i);
                        }
                    }}
            };
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

        void Space::focus(Node *const &node) {
            for (auto i = node; i->_parent && i->_parent->_active_iter != i->_iter_parent; ({
                i->_parent->_active_iter = i->_iter_parent;
                i = i->_parent;
            }));
            auto leaf = node->getActiveLeaf();
            XSetInputFocus(_display, leaf->_window, RevertToNone, CurrentTime);
        }

        node::Branch *Space::join(wm::matrix::Node *const &node, wm::matrix::Node *const &target, const wm::matrix::FB &fb) {
            if (!node->_parent) return target->_receive(node, fb);
            else return nullptr;
        }

        Node::Node(Space *const &space) : _space(space) {
            _parent = nullptr;
        }

        Node::~Node() = default;

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

            node::Branch *Branch::_receive(wm::matrix::Node *const &node, const wm::matrix::FB &fb) {
                node->_iter_parent = _children.insert(fb ? std::next(_active_iter) : _active_iter, node);
                node->_parent = this;
                return this;
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