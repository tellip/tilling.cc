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

        struct {
            uint32_t background;
            struct {
                uint16_t width;
                struct {
                    uint32_t focused, locked;
                } color;
            } border;
        } _attributes;
        uint16_t _root_width, _root_height;
        HV _root_hv;

        Node *_root, *_view;
        node::Leaf *_active;
        std::unordered_map<xcb_window_t, node::Leaf *> _leaves;

        PointerCoordinate _pointer_coordinate;

        bool _manual_refreshing;
        bool _exiting;
        bool _active_locked;
    public:
        const std::unordered_map<
                int,
                std::function<void(xcb_generic_event_t *const &)>
        > event_handlers;

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

        void toggleLockingActive();

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
        Space &_space;

        node::Branch *_parent;

        std::list<Node *>::iterator _parent_iter;
        Attribute _attribute;

        explicit Node(Space &);

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

            Leaf(Space &, const xcb_window_t &);

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

            explicit Branch(Space &);

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
    const std::unordered_map<std::string, std::function<void(tree::Space &)>> command_handlers = {
            {"exit",                  [](tree::Space &space) {
                space.exit();
            }},

            {"refresh",               [](tree::Space &space) {
                space.refresh(true);
            }},

            {"focus-up",              [](tree::Space &space) {
                space.focus(tree::HV::VERTICAL, tree::FB::BACKWARD);
            }},
            {"focus-right",           [](tree::Space &space) {
                space.focus(tree::HV::HORIZONTAL, tree::FB::FORWARD);
            }},
            {"focus-down",            [](tree::Space &space) {
                space.focus(tree::HV::VERTICAL, tree::FB::FORWARD);
            }},
            {"focus-left",            [](tree::Space &space) {
                space.focus(tree::HV::HORIZONTAL, tree::FB::BACKWARD);
            }},

            {"reorder-up",            [](tree::Space &space) {
                space.reorder(tree::HV::VERTICAL, tree::FB::BACKWARD);
            }},
            {"reorder-right",         [](tree::Space &space) {
                space.reorder(tree::HV::HORIZONTAL, tree::FB::FORWARD);
            }},
            {"reorder-down",          [](tree::Space &space) {
                space.reorder(tree::HV::VERTICAL, tree::FB::FORWARD);
            }},
            {"reorder-left",          [](tree::Space &space) {
                space.reorder(tree::HV::HORIZONTAL, tree::FB::BACKWARD);
            }},

            {"reparent-up",           [](tree::Space &space) {
                space.reparent(tree::HV::VERTICAL, tree::FB::BACKWARD);
            }},
            {"reparent-right",        [](tree::Space &space) {
                space.reparent(tree::HV::HORIZONTAL, tree::FB::FORWARD);
            }},
            {"reparent-down",         [](tree::Space &space) {
                space.reparent(tree::HV::VERTICAL, tree::FB::FORWARD);
            }},
            {"reparent-left",         [](tree::Space &space) {
                space.reparent(tree::HV::HORIZONTAL, tree::FB::BACKWARD);
            }},

            {"reorganize-up",         [](tree::Space &space) {
                space.reorganize(tree::HV::VERTICAL, tree::FB::BACKWARD);
            }},
            {"reorganize-right",      [](tree::Space &space) {
                space.reorganize(tree::HV::HORIZONTAL, tree::FB::FORWARD);
            }},
            {"reorganize-down",       [](tree::Space &space) {
                space.reorganize(tree::HV::VERTICAL, tree::FB::FORWARD);
            }},
            {"reorganize-left",       [](tree::Space &space) {
                space.reorganize(tree::HV::HORIZONTAL, tree::FB::BACKWARD);
            }},

            {"view-in",               [](tree::Space &space) {
                space.viewResize(tree::FB::FORWARD);
            }},
            {"view-out",              [](tree::Space &space) {
                space.viewResize(tree::FB::BACKWARD);
            }},
            {"view-leaf",             [](tree::Space &space) {
                space.viewExtreme(tree::FB::FORWARD);
            }},
            {"view-root",             [](tree::Space &space) {
                space.viewExtreme(tree::FB::BACKWARD);
            }},
            {"view-forward",          [](tree::Space &space) {
                space.viewMove(tree::FB::FORWARD);
            }},
            {"view-backward",         [](tree::Space &space) {
                space.viewMove(tree::FB::BACKWARD);
            }},

            {"transpose",             [](tree::Space &space) {
                space.transpose();
            }},

            {"close-window",          [](tree::Space &space) {
                space.closeActive(false);
            }},
            {"kill-window",           [](tree::Space &space) {
                space.closeActive(true);
            }},
            {"toggle-locking-active", [](tree::Space &space) {
                space.toggleLockingActive();
            }}
    };

    struct CommandHandler : server::Handler<std::string> {
        Space &space;

        explicit CommandHandler(Space &space) : space(space) {}

        inline void handle(const std::string &task) const final {
            auto i = command_handlers.find(task);
            if (i != command_handlers.cend()) i->second(space);
        };
    };

    struct EventHandler : server::Handler<xcb_generic_event_t *> {
        Space &space;

        explicit EventHandler(Space &space) : space(space) {}

        inline void handle(xcb_generic_event_t *const &event) const final {
            auto i = space.event_handlers.find(event->response_type & ~0x80);
            if (i != space.event_handlers.cend()) i->second(event);
        }
    };

    const auto tree = [](xcb_connection_t *const &x_connection, xcb_screen_t *const &x_default_screen, const auto &breakLoop, const auto &callback) {
        auto space = Space(x_connection, x_default_screen, breakLoop);
        space.refresh();
        callback(
                CommandHandler(space),
                root_event_mask,
                leaf_event_mask,
                EventHandler(space)
        );
    };
}