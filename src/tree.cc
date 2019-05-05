#include "tree.h"

namespace project::tree {
    PointerCoordinate::PointerCoordinate(xcb_connection_t *const &x_connection,
                                         xcb_screen_t *const &x_default_screen) : _x_connection(x_connection),
                                                                                  _x_default_screen(
                                                                                          x_default_screen) {
        _x = _y = -1;
    }

    void PointerCoordinate::refresh() {
        auto reply = xcb_query_pointer_reply(
                _x_connection,
                xcb_query_pointer(
                        _x_connection,
                        _x_default_screen->root
                ),
                nullptr
        );
        if (!reply) helper::error("xcb_query_pointer_reply");
        _root_x = reply->root_x;
        _root_y = reply->root_y;
        free(reply);
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

    Space::Space(xcb_connection_t *const &x_connection, xcb_screen_t *const &x_default_screen, const std::function<void()> &break_) :
            _breakLoop(break_),
            _x_connection(x_connection),
            _x_default_screen(x_default_screen),
            _xia_protocols(_internAtom("WM_PROTOCOLS")),
            _xia_delete_window(_internAtom("WM_DELETE_WINDOW")),
            _mask_layer(xcb_generate_id(x_connection)),
            event_handlers(
                    {
                            {XCB_MAP_NOTIFY,       [&](xcb_generic_event_t *const &event) {
                                auto map_notify = (xcb_map_notify_event_t *) event;
                                if (!map_notify->override_redirect) {
                                    if (map_notify->window != _mask_layer && _leaves.find(map_notify->window) == _leaves.end()) {
                                        auto leaf = new node::Leaf(this, map_notify->window);

                                        std::list<std::function<void()>> fl;
                                        if (!_active) {
                                            fl.emplace_back([&]() {
                                                _root = _view = leaf;
                                            });
                                            leaf->_configure(
                                                    {
                                                            _root_hv,
                                                            static_cast<int16_t>(-_border_width),
                                                            static_cast<int16_t>(-_border_width),
                                                            static_cast<uint16_t>(_root_width + _border_width * 2),
                                                            static_cast<uint16_t>(_root_height + _border_width * 2)
                                                    }
                                            );
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
                                        xcb_flush(_x_connection);
                                        _pointer_coordinate.record();
                                    } else helper::error("\"_leaves.find(event.xmap.window) != _leaves.end()\"");
                                }
                            }},
                            {XCB_UNMAP_NOTIFY,     [&](xcb_generic_event_t *const &event) {
                                auto unmap_notify = (xcb_unmap_notify_event_t *) event;
                                auto i = _leaves.find(unmap_notify->window);
                                if (i != _leaves.end()) {
                                    auto leaf = i->second;
                                    if (leaf == _active) {
                                        if (leaf->_parent) {
                                            auto j = leaf->_parent_iter;
                                            if (j == leaf->_parent->_children.begin()) j = std::next(j);
                                            else j = std::prev(j);
                                            auto sibling = *j;
                                            _activate(sibling);
                                            _active = sibling->_activeLeaf(FB::FORWARD);
                                            _active->_focus(true);
                                        } else _active = nullptr;
                                    }
                                    Node *rest;
                                    std::list<std::function<void()>> fl;
                                    if (leaf == _view ||
                                        (leaf->_parent == _view && leaf->_parent->_children.size() <= 2)) {
                                        if (leaf->_parent) leaf->_parent->_configure(_view->_attribute);
                                        fl.emplace_back([&]() {
                                            if (rest) rest->_raise();
                                            _view = rest;
                                        });
                                        if (_root == _view) {
                                            fl.emplace_back([&]() {
                                                _root = _view;
                                            });
                                        }
                                    }
                                    rest = _quit(leaf);
                                    if (rest) rest->_refresh();
                                    delete leaf;
                                    for (auto j = fl.cbegin(); j != fl.cend(); ({
                                        (*j)();
                                        j++;
                                    }));
                                    xcb_flush(_x_connection);
                                    _pointer_coordinate.record();

                                    if (_exiting) closeActive(false);
                                }
                            }},
                            {XCB_CONFIGURE_NOTIFY, [&](xcb_generic_event_t *const &event) {
                                auto configure_notify = (xcb_configure_notify_event_t *) event;
                                auto i = _leaves.find(configure_notify->window);
                                if (i != _leaves.end()) {
                                    auto leaf = i->second;
                                    {
                                        auto reply = xcb_get_geometry_reply(_x_connection,
                                                                            xcb_get_geometry(_x_connection,
                                                                                             leaf->_window),
                                                                            nullptr);
                                        if (!reply) helper::error("xcb_get_geometry_reply");
                                        if (reply->x != leaf->_attribute.x)
                                            xcb_configure_window(_x_connection, leaf->_window, XCB_CONFIG_WINDOW_X,
                                                                 ({
                                                                     int16_t values[] = {leaf->_attribute.x};
                                                                     values;
                                                                 }));
                                        if (reply->y != leaf->_attribute.y)
                                            xcb_configure_window(_x_connection, leaf->_window, XCB_CONFIG_WINDOW_Y,
                                                                 ({
                                                                     int16_t values[] = {leaf->_attribute.y};
                                                                     values;
                                                                 }));
                                        if (reply->width != leaf->_attribute.width - _border_width * 2)
                                            xcb_configure_window(_x_connection, leaf->_window, XCB_CONFIG_WINDOW_WIDTH, ({
                                                uint16_t values[] = {(uint16_t) (leaf->_attribute.width - _border_width * 2)};
                                                values;
                                            }));
                                        if (reply->height != leaf->_attribute.height - _border_width * 2)
                                            xcb_configure_window(_x_connection, leaf->_window, XCB_CONFIG_WINDOW_HEIGHT, ({
                                                uint16_t values[] = {(uint16_t) (leaf->_attribute.height - _border_width * 2)};
                                                values;
                                            }));
                                    }

                                    {
                                        Node *ancestor = leaf;
                                        while (ancestor && ancestor != _view) ancestor = ancestor->_parent;

                                        xcb_window_t low;
                                        auto reply = xcb_query_tree_reply(_x_connection,
                                                                          xcb_query_tree(_x_connection,
                                                                                         _x_default_screen->root),
                                                                          nullptr);
                                        if (!reply) helper::error("xcb_query_tree_reply");
                                        auto children = xcb_query_tree_children(reply);
                                        for (auto j = 0; j < reply->children_len; ({
                                            auto child = children[j++];
                                            if (child == leaf->_window || child == _mask_layer) {
                                                low = child;
                                                j = reply->children_len;
                                            }
                                        }));
                                        free(reply);

                                        if (low == _mask_layer && ancestor != _view)
                                            xcb_configure_window(_x_connection, leaf->_window, XCB_CONFIG_WINDOW_STACK_MODE, ({
                                                uint32_t values[] = {XCB_STACK_MODE_BELOW};
                                                values;
                                            }));
                                        else if (low == leaf->_window && ancestor == _view)
                                            xcb_configure_window(_x_connection, leaf->_window, XCB_CONFIG_WINDOW_STACK_MODE, ({
                                                uint32_t values[] = {XCB_STACK_MODE_ABOVE};
                                                values;
                                            }));
                                    }
                                    xcb_flush(_x_connection);
                                }
                            }},
                            {XCB_ENTER_NOTIFY,     [&](xcb_generic_event_t *const &event) {
                                auto enter_notify = (xcb_enter_notify_event_t *) event;
                                auto i = _leaves.find(enter_notify->event);
                                if (i != _leaves.end()) {
                                    auto leaf = i->second;
                                    if ((!_manual_refreshing ? true : (_manual_refreshing = false)) && _active != leaf && !_pointer_coordinate.check()) {
                                        for (Node *j = leaf; j->_parent; ({
                                            j->_parent->_iter_active = j->_parent_iter;
                                            j = j->_parent;
                                        }));

                                        if (_active) _active->_focus(false);
                                        leaf->_focus(true);
                                        _active = leaf;
                                        xcb_flush(_x_connection);
                                    }
                                }
                            }},
                            {XCB_FOCUS_IN,         [&](xcb_generic_event_t *const &event) {
                                auto focus_in = (xcb_focus_in_event_t *) event;
                                auto i = _leaves.find(focus_in->event);
                                if (i != _leaves.end()) {
                                    auto leaf = i->second;
                                    if (leaf != _active && _active) {
                                        xcb_set_input_focus(_x_connection, XCB_INPUT_FOCUS_PARENT, _active->_window, XCB_CURRENT_TIME);
                                        xcb_flush(_x_connection);
                                    }
                                }
                            }}
                    }
            ),
            _pointer_coordinate(x_connection, x_default_screen) {
        xcb_create_window(
                x_connection,
                XCB_COPY_FROM_PARENT,
                _mask_layer,
                _x_default_screen->root,
                0, 0, 1, 1, 0,
                XCB_WINDOW_CLASS_INPUT_OUTPUT,
                _x_default_screen->root_visual,
                0, nullptr
        );
        xcb_map_window(_x_connection, _mask_layer);
        _root = _view = _active = nullptr;
        _exiting = false;
        _manual_refreshing = false;
        refresh();
    }

    Space::~Space() {
        delete _root;
        _root = _view = _active = nullptr;
    }

    xcb_atom_t Space::_internAtom(const char *const &msg) {
        auto reply = xcb_intern_atom_reply(_x_connection, xcb_intern_atom(_x_connection, 0, (uint16_t) strlen(msg), msg), nullptr);
        if (!reply) helper::error("xcb_intern_atom_reply");
        auto atom = reply->atom;
        free(reply);
        return atom;
    }

    uint32_t Space::_colorPixel(const std::string &cn) {
        auto reply = xcb_alloc_named_color_reply(
                _x_connection,
                xcb_alloc_named_color(
                        _x_connection,
                        _x_default_screen->default_colormap,
                        cn.size(),
                        cn.c_str()
                ),
                nullptr
        );
        if (!reply) helper::error("xcb_alloc_named_color_reply");
        auto pixel = reply->pixel;
        free(reply);
        return pixel;
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
                    if (grand_parent->_iter_active == parent->_parent_iter)
                        grand_parent->_iter_active = sibling->_parent_iter;
                    grand_parent->_children.erase(parent->_parent_iter);
                }
                sibling->_parent = grand_parent;
                sibling->_configure(parent->_attribute);
                delete parent;
                return sibling;
            } else if (parent->_children.size() > 1) {
                parent->_configureChildren();
                return parent;
            } else {
                helper::error("\"parent->_children.size()<1\"");
                return nullptr;
            }
        } else return nullptr;
    }

    void Space::_activate(Node *const &node) {
        for (auto i = node; i->_parent && i->_parent->_iter_active != i->_parent_iter; ({
            i->_parent->_iter_active = i->_parent_iter;
            i = i->_parent;
        }));
    }

    void Space::refresh(const bool &manual) {
        if (manual) _manual_refreshing = true;

        static bool called = false;
        static const auto config_path = getenv(config::config_path_evn);

        {
            std::stringstream ss;
            std::ifstream ifs(config_path);
            ss << ifs.rdbuf();
            ifs.close();
            using json=json::Json<>;
            auto config = json::parse(ss.str()).as_object();
            _border_width = static_cast<uint16_t>(config["border_width"].as_number());
            _normal_pixel = _colorPixel(config["color"].as_object()["normal"].as_string());
            _focused_pixel = _colorPixel(config["color"].as_object()["focused"].as_string());
        }

        auto reply = xcb_get_geometry_reply(_x_connection, xcb_get_geometry(_x_connection, _x_default_screen->root), nullptr);
        _root_width = reply->width;
        _root_height = reply->height;
        free(reply);
        if (!called) {
            called = true;
            _root_hv = (HV) (_root_width >= _root_height);
        }

        xcb_change_window_attributes(_x_connection, _mask_layer, XCB_CW_BACK_PIXEL, ({
            uint32_t values[] = {_normal_pixel};
            values;
        }));
        xcb_configure_window(_x_connection, _mask_layer, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, ({
            uint16_t values[] = {_root_width, _root_height};
            values;
        }));
        xcb_configure_window(_x_connection, _mask_layer, XCB_CONFIG_WINDOW_STACK_MODE, ({
            uint32_t values[] = {_root_width, _root_height, XCB_STACK_MODE_ABOVE};
            values;
        }));

        if (_view) {
            _view->_configure(
                    {
                            _root_hv,
                            static_cast<int16_t>(-_border_width),
                            static_cast<int16_t>(-_border_width),
                            static_cast<uint16_t>(_root_width + _border_width * 2),
                            static_cast<uint16_t>(_root_height + _border_width * 2)
                    }
            );
            _view->_refresh();
            _view->_raise();
        }
        xcb_flush(_x_connection);
    }

    void Space::exit() {
        _exiting = true;
        closeActive(false);
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
            xcb_flush(_x_connection);
        }
    }

    void Space::reorder(const HV &hv, const FB &fb) {
        if (_active && _active != _view && _active->_parent->_attribute.hv == hv) {
            _move(_active, std::next(_active->_parent_iter, lround(0.5 + 1.5 * (fb ? 1 : -1))));
            _activate(_active);
            _active->_parent->_refresh();
            _pointer_coordinate.record();
            xcb_flush(_x_connection);
        }
    }

    void Space::reparent(const HV &hv, const FB &fb) {
        if (_active && _active != _view) {
            if (hv == _active->_parent->_attribute.hv) {
                auto i = std::next(_active->_parent_iter, fb ? 1 : -1);
                if (i == _active->_parent->_children.end()) i = std::next(i, fb ? 1 : -1);
                auto sibling = dynamic_cast<node::Branch *>(*i);
                if (sibling) {
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
                    if (_active->_parent == _view && _active->_parent->_children.size() <= 2) {
                        fl.emplace_back([&]() {
                            _view = _active->_parent;
                        });
                        if (_root == _view) {
                            fl.emplace_back([&]() {
                                _root = _view;
                            });
                        }
                    }

                    _quit(_active);
                    _join(_active, sibling, FB(!fb));
                    _activate(_active);

                    for (auto j = fl.cbegin(); j != fl.cend(); ({
                        (*j)();
                        j++;
                    }));

                    xcb_flush(_x_connection);
                    _pointer_coordinate.record();
                }
            } else if (_active->_parent != _view) {
                auto rest = _quit(_active);
                _activate(rest);
                _join(_active, rest->_parent, fb);

                _activate(_active);
                _active->_parent->_refresh();

                xcb_flush(_x_connection);
                _pointer_coordinate.record();
            }
        }
    }

    void Space::reorganize(const HV &hv, const FB &fb) {
        if (_active && _active != _view) {
            if (hv == _active->_parent->_attribute.hv) {
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
                if (_active->_parent == _view && _active->_parent->_children.size() <= 2) {
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
                _quit(sibling);
                _join(sibling, _active, fb);
                _activate(_active);

                for (auto j = fl.cbegin(); j != fl.cend(); ({
                    (*j)();
                    j++;
                }));
            } else {
                std::list<std::function<void()>> fl;

                if (_active->_parent == _view) {
                    fl.emplace_back([&]() {
                        _view = _active->_parent;
                    });
                    if (_root == _view) {
                        fl.emplace_back([&]() {
                            _root = _view;
                        });
                    }
                }

                auto attribute = _active->_parent->_attribute;
                if (_root == _active->_parent) {
                    auto rest = _quit(_active);
                    _join(rest, _active, FB(!fb));
                } else {
                    auto rest = _quit(_active);
                    _activate(rest);
                    _join(_active, rest->_parent, fb);
                    _quit(rest);
                    _join(rest, _active, FB(!fb));
                }
                _active->_parent->_configure(attribute);

                for (auto j = fl.cbegin(); j != fl.cend(); ({
                    (*j)();
                    j++;
                }));
                _activate(_active);
                _active->_parent->_refresh();
            }
            xcb_flush(_x_connection);
            _pointer_coordinate.record();
        }
    }

    void Space::viewResize(const FB &fb) {
        if (_view) {
            auto new_view = (fb ? _view->_activeChild() : _view->_parent);
            if (new_view) {
                new_view->_configure(_view->_attribute);
                xcb_configure_window(_x_connection, _mask_layer, XCB_CONFIG_WINDOW_STACK_MODE, ({
                    uint32_t values[] = {XCB_STACK_MODE_ABOVE};
                    values;
                }));
                new_view->_refresh();
                new_view->_raise();
                _view = new_view;
                xcb_flush(_x_connection);
                _pointer_coordinate.record();
            }
        }
    }

    void Space::viewMove(const FB &fb) {
        if (_view && _view->_parent) {
            auto i = std::next(_view->_parent_iter, fb ? 1 : -1);
            if (i == _view->_parent->_children.end()) i = std::next(i, fb ? 1 : -1);
            auto new_view = *i;

            new_view->_configure(_view->_attribute);
            xcb_configure_window(_x_connection, _mask_layer, XCB_CONFIG_WINDOW_STACK_MODE, ({
                uint32_t values[] = {XCB_STACK_MODE_ABOVE};
                values;
            }));
            new_view->_refresh();
            new_view->_raise();
            _activate(new_view);
            auto new_active = new_view->_activeLeaf(FB(!fb));
            _active->_focus(false);
            new_active->_focus(true);

            _view = new_view;
            _active = new_active;
            _pointer_coordinate.record();
        }
    }

    void Space::viewExtreme(const FB &fb) {
        auto new_view = (fb ? _active : _root);
        if (new_view) {
            new_view->_configure(_view->_attribute);
            xcb_configure_window(_x_connection, _mask_layer, XCB_CONFIG_WINDOW_STACK_MODE, ({
                uint32_t values[] = {XCB_STACK_MODE_ABOVE};
                values;
            }));
            new_view->_refresh();
            new_view->_raise();
            _view = new_view;
            _pointer_coordinate.record();
        }
    }

    void Space::transpose() {
        if (_view) {
            auto attribute = _view->_attribute;
            attribute.hv = _root_hv = HV(!_root_hv);
            _view->_configure(attribute);
            _view->_refresh();
            xcb_flush(_x_connection);
        }
    }

    void Space::closeActive(const bool &force) {
        if (_active) {
            if (force) xcb_destroy_window(_x_connection, _active->_window);
            else {
                auto client_message = new xcb_client_message_event_t();
                client_message->response_type = XCB_CLIENT_MESSAGE;
                client_message->window = _active->_window;
                client_message->type = _xia_protocols;
                client_message->format = 32;
                client_message->data.data32[0] = _xia_delete_window;
                client_message->data.data32[1] = XCB_CURRENT_TIME;
                xcb_send_event(_x_connection, 0, _active->_window, XCB_EVENT_MASK_NO_EVENT,
                               (const char *) client_message);
                delete client_message;
            }
            xcb_flush(_x_connection);
        } else if (_exiting) _breakLoop();
    }

    Node::Node(Space *const &space) : _space(space) {
        _parent = nullptr;
    }

    Node::~Node() = default;

    void Node::_configure(const Attribute &attribute) {
        _attribute = attribute;
    }

    namespace node {
        Leaf::Leaf(Space *const &space, const xcb_window_t &window) :
                Node(space),
                _window(window),
                _leaves_iter(space->_leaves.insert(std::make_pair(window, this)).first) {
            _focused = false;
            auto reply = xcb_get_geometry_reply(_space->_x_connection,
                                                xcb_get_geometry(_space->_x_connection, _window), nullptr);
            if (!reply) helper::error("xcb_get_geometry_reply");
            _attribute.x = reply->x;
            _attribute.y = reply->y;
            _attribute.width = (uint16_t) (reply->width + _space->_border_width * 2);
            _attribute.height = (uint16_t) (reply->height + _space->_border_width * 2);

            xcb_configure_window(
                    _space->_x_connection,
                    window,
                    XCB_CONFIG_WINDOW_BORDER_WIDTH,
                    ({
                        uint16_t values[] = {_space->_border_width};
                        values;
                    })
            );
            xcb_change_window_attributes(space->_x_connection, window, XCB_CW_EVENT_MASK, ({
                uint32_t values[] = {leaf_event_mask};
                values;
            }));
        }

        Leaf::~Leaf() {
            _space->_leaves.erase(_leaves_iter);
        }

        void Leaf::_refresh() {
            xcb_configure_window(
                    _space->_x_connection,
                    _window,
                    XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT | XCB_CONFIG_WINDOW_BORDER_WIDTH,
                    ({
                        int32_t values[] = {_attribute.x, _attribute.y, _attribute.width - _space->_border_width * 2, _attribute.height - _space->_border_width * 2, _space->_border_width};
                        values;
                    })
            );
            _focus(_focused);
        }

        void Leaf::_raise() {
            xcb_configure_window(_space->_x_connection, _window, XCB_CONFIG_WINDOW_STACK_MODE, ({
                uint32_t values[] = {XCB_STACK_MODE_ABOVE};
                values;
            }));
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
            node->_parent_iter = new_parent->_children.insert(
                    fb ? new_parent->_children.end() : new_parent->_children.begin(), node);
            node->_parent = new_parent;
            new_parent->_configure(_attribute);
            return new_parent;
        }

        void Leaf::_focus(const bool &focused) {
            _focused = focused;
            if (focused) {
                xcb_change_window_attributes(_space->_x_connection, _window, XCB_CW_BORDER_PIXEL, ({
                    uint32_t values[] = {_space->_focused_pixel};
                    values;
                }));
                xcb_set_input_focus(_space->_x_connection, XCB_INPUT_FOCUS_PARENT, _window, XCB_CURRENT_TIME);
            } else
                xcb_change_window_attributes(_space->_x_connection, _window, XCB_CW_BORDER_PIXEL, ({
                    uint32_t values[] = {_space->_normal_pixel};
                    values;
                }));
        }

        Branch::Branch(Space *const &space) : Node(space) {}

        Branch::~Branch() {
            for (auto i = _children.cbegin(); i != _children.cend(); delete *i++);
            _children.clear();
        }

        void Branch::_configure(const Attribute &attribute) {
            Node::_configure(attribute);
            _configureChildren();
        }

        void Branch::_refresh() {
            for (auto i = _children.cbegin(); i != _children.cend(); (*i++)->_refresh());
        }

        void Branch::_raise() {
            for (auto i = _children.cbegin(); i != _children.cend(); (*i++)->_raise());
        }

        Leaf *Branch::_activeLeaf(const FB &fb) {
            if (_iter_active == _children.end())
                _iter_active = (fb ? std::prev(_children.end()) : _children.begin());
            return (*_iter_active)->_activeLeaf(fb);
        }

        Node *Branch::_activeChild() {
            return *_iter_active;
        }

        Branch *Branch::_receive(Node *const &node, const FB &fb) {
            node->_parent_iter = _children.insert(
                    _iter_active == _children.end() ? (fb ? _children.end() : _children.begin()) : (fb ? std::next(
                            _iter_active) : _iter_active), node);
            node->_parent = this;
            _configureChildren();
            return this;
        }

        void Branch::_configureChildren() {
            switch (_attribute.hv) {
                case HV::HORIZONTAL: {
                    auto s = (int16_t) (_attribute.x + _space->_border_width);
                    auto d = (uint16_t) ((_attribute.width - _space->_border_width * 2) / _children.size());
                    for (auto i = _children.cbegin(); i != std::prev(_children.cend()); ({
                        (*i++)->_configure(
                                {
                                        HV(!_attribute.hv),
                                        s, (int16_t) (_attribute.y + _space->_border_width),
                                        d, (uint16_t) (_attribute.height - _space->_border_width * 2)
                                }
                        );
                        s += d;
                    }));
                    (*std::prev(_children.cend()))->_configure(
                            {
                                    HV(!_attribute.hv),
                                    s, (int16_t) (_attribute.y + _space->_border_width),
                                    (uint16_t) (_attribute.x + _attribute.width - _space->_border_width - s),
                                    (uint16_t) (_attribute.height - _space->_border_width * 2)
                            }
                    );
                    break;
                }
                case HV::VERTICAL: {
                    auto s = (int16_t) (_attribute.y + _space->_border_width);
                    auto d = (uint16_t) ((_attribute.height - _space->_border_width * 2) / _children.size());
                    for (auto i = _children.cbegin(); i != std::prev(_children.cend()); ({
                        (*i++)->_configure(
                                {
                                        HV(!_attribute.hv),
                                        (int16_t) (_attribute.x + _space->_border_width), s,
                                        (uint16_t) (_attribute.width - _space->_border_width * 2), d
                                }
                        );
                        s += d;
                    }));
                    (*std::prev(_children.cend()))->_configure(
                            {
                                    HV(!_attribute.hv),
                                    (int16_t) (_attribute.x + _space->_border_width), s,
                                    (uint16_t) (_attribute.width - _space->_border_width * 2),
                                    (uint16_t) (_attribute.y + _attribute.height - _space->_border_width - s)
                            }
                    );
                }
            }
        }
    }
}