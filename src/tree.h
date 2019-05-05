#pragma once

#include "wm.h"

namespace wm::tree {
    class PointerCoordinate {
        xcb_connection_t *const _x_connection;
        xcb_screen_t *const _x_default_screen;

        int16_t _x, _y;

        int16_t _root_x, _root_y;
    public:
        explicit PointerCoordinate(xcb_connection_t *const &, xcb_screen_t *const &);

        void refresh();

        void record();

        bool check();
    };

    class Space {
        const std::function<void()> _breakLoop;
        xcb_connection_t *const _x_connection;
        xcb_screen_t *const _x_default_screen;
        const xcb_atom_t _xia_protocols, _xia_delete_window;
        const xcb_window_t _mask_layer;

        uint16_t _border_width;
        uint32_t _normal_pixel, _focused_pixel;
        uint16_t _root_width, _root_height;
        HV _root_hv;

        Node *_root, *_view;
        node::Leaf *_active;
        std::unordered_map<xcb_window_t, node::Leaf *> _leaves;

        PointerCoordinate _pointer_coordinate;

        bool _manual_refreshing;
        bool _exiting;
    public:
        const server::EventHandlers event_handlers;

        Space(xcb_connection_t *const &, xcb_screen_t *const &, const std::function<void()> &);

        ~Space();

    private:
        xcb_atom_t _internAtom(const char *const &);

        uint32_t _colorPixel(const std::string &);

        node::Branch *_join(Node *const &, Node *const &, const FB &);

        void _move(Node *const &, const std::list<Node *>::iterator &);

        Node *_quit(Node *const &);

        void _activate(Node *const &);

    public:
        void refresh(const bool & = false);

        void exit();

        void focus(const HV &, const FB &);

        void reorder(const HV &, const FB &);

        void reparent(const HV &, const FB &);

        void reorganize(const HV &, const FB &);

        void viewResize(const FB &);

        void viewMove(const FB &);

        void viewExtreme(const FB &);

        void transpose();

        void closeActive(const bool &);

        friend Node;
        friend node::Branch;
        friend node::Leaf;
    };

    struct Attribute {
        HV hv;
        int16_t x, y;
        uint16_t width, height;
    };

    class Node {
        Space *const _space;

        node::Branch *_parent;

        std::list<Node *>::iterator _parent_iter;
        Attribute _attribute;

        explicit Node(Space *const &);

        virtual ~Node() = 0;

        virtual void _configure(const Attribute &);

        virtual void _refresh() = 0;

        virtual void _raise() = 0;

        virtual node::Leaf *_activeLeaf(const FB &) = 0;

        virtual Node *_activeChild() = 0;

        virtual node::Branch *_receive(Node *const &, const FB &) = 0;

        friend Space;
        friend node::Leaf;
        friend node::Branch;
    };

    namespace node {
        class Leaf : public Node {
            const xcb_window_t _window;
            const std::unordered_map<xcb_window_t, Leaf *>::iterator _leaves_iter;
            bool _focused;

            Leaf(Space *const &, const xcb_window_t &);

            ~Leaf() final;

            void _refresh() final;

            void _raise() final;

            node::Leaf *_activeLeaf(const FB &) final;

            Node *_activeChild() final;

            node::Branch *_receive(Node *const &, const FB &) final;

            void _focus(const bool &);

            friend Space;
        };

        class Branch : public Node {
            std::list<Node *> _children;
            std::list<Node *>::iterator _iter_active;

            explicit Branch(Space *const &);

            ~Branch() final;

            void _configure(const Attribute &) final;

            void _refresh() final;

            void _raise() final;

            node::Leaf *_activeLeaf(const FB &) final;

            Node *_activeChild() final;

            node::Branch *_receive(Node *const &, const FB &) final;

            void _configureChildren();

            friend Space;
            friend Leaf;
        };
    }

    const uint32_t root_event_mask = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    const uint32_t leaf_event_mask = XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_FOCUS_CHANGE;

    auto tree = [](xcb_connection_t *const &x_connection, xcb_screen_t *const &x_default_screen, const auto &breakLoop, const auto &callback) {
        auto space = Space(x_connection, x_default_screen, breakLoop);
        space.refresh();
        server::CommandHandlers command_handlers = {
                {"exit",             [&]() {
                    space.exit();
                }},

                {"refresh",          [&]() {
                    space.refresh(true);
                }},

                {"focus-up",         [&]() {
                    space.focus(HV::VERTICAL, FB::BACKWARD);
                }},
                {"focus-right",      [&]() {
                    space.focus(HV::HORIZONTAL, FB::FORWARD);
                }},
                {"focus-down",       [&]() {
                    space.focus(HV::VERTICAL, FB::FORWARD);
                }},
                {"focus-left",       [&]() {
                    space.focus(HV::HORIZONTAL, FB::BACKWARD);
                }},

                {"reorder-up",       [&]() {
                    space.reorder(HV::VERTICAL, FB::BACKWARD);
                }},
                {"reorder-right",    [&]() {
                    space.reorder(HV::HORIZONTAL, FB::FORWARD);
                }},
                {"reorder-down",     [&]() {
                    space.reorder(HV::VERTICAL, FB::FORWARD);
                }},
                {"reorder-left",     [&]() {
                    space.reorder(HV::HORIZONTAL, FB::BACKWARD);
                }},

                {"reparent-up",      [&]() {
                    space.reparent(HV::VERTICAL, FB::BACKWARD);
                }},
                {"reparent-right",   [&]() {
                    space.reparent(HV::HORIZONTAL, FB::FORWARD);
                }},
                {"reparent-down",    [&]() {
                    space.reparent(HV::VERTICAL, FB::FORWARD);
                }},
                {"reparent-left",    [&]() {
                    space.reparent(HV::HORIZONTAL, FB::BACKWARD);
                }},

                {"reorganize-up",    [&]() {
                    space.reorganize(HV::VERTICAL, FB::BACKWARD);
                }},
                {"reorganize-right", [&]() {
                    space.reorganize(HV::HORIZONTAL, FB::FORWARD);
                }},
                {"reorganize-down",  [&]() {
                    space.reorganize(HV::VERTICAL, FB::FORWARD);
                }},
                {"reorganize-left",  [&]() {
                    space.reorganize(HV::HORIZONTAL, FB::BACKWARD);
                }},

                {"view-in",          [&]() {
                    space.viewResize(FB::FORWARD);
                }},
                {"view-out",         [&]() {
                    space.viewResize(FB::BACKWARD);
                }},
                {"view-leaf",        [&]() {
                    space.viewExtreme(FB::FORWARD);
                }},
                {"view-root",        [&]() {
                    space.viewExtreme(FB::BACKWARD);
                }},
                {"view-forward",     [&]() {
                    space.viewMove(FB::FORWARD);
                }},
                {"view-backward",    [&]() {
                    space.viewMove(FB::BACKWARD);
                }},

                {"transpose",        [&]() {
                    space.transpose();
                }},

                {"close-window",     [&]() {
                    space.closeActive(false);
                }},
                {"kill-window",      [&]() {
                    space.closeActive(true);
                }}
        };
        callback(
                command_handlers,
                root_event_mask,
                leaf_event_mask,
                space.event_handlers
        );
    };
}