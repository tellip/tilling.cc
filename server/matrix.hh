#pragma once

#include "main.hh"

namespace wm {
    namespace matrix {
        class PointerCoordinate {
            Display *const _display;

            int _x, _y;

            Window _root, _child;
            int _root_x, _root_y, _win_x, _win_y;
            unsigned int _mask;
        public:
            explicit PointerCoordinate(Display *const &);

            void refresh();

            void record();

            bool check();
        };

        class Space {
            const std::function<void()> _breakLoop;
            Display *const _display;
            const unsigned long _normal_pixel, _focus_pixel;
            const Atom _xia_protocols, _xia_delete_window;
            const Window _mask_layer;

            unsigned int _display_width, _display_height;
            HV _display_hv;

            Node *_root, *_view;
            node::Leaf *_active;
            std::unordered_map<Window, node::Leaf *> _leaves;

            PointerCoordinate _pointer_coordinate;

        public:
            const server::EventHandlers event_handlers;

            Space(Display *const &, const std::function<void()> &);

        private:
            unsigned long _colorPixel(const char *const &);

            node::Branch *_join(Node *const &, Node *const &, const FB &);

            void _move(Node *const &, const std::list<Node *>::iterator &);

            Node *_quit(Node *const &);

            void _activate(Node *const &);

        public:
            void refresh();

            void exit();

            void focus(const HV &, const FB &);

            void move(const HV &, const FB &);

            void reparent(const HV &, const FB &);

            void viewResize(const FB &);

            void viewMove(const FB &);

            void transpose();

            friend Node;
            friend node::Branch;
            friend node::Leaf;
        };

        struct Attribute {
            HV hv;
            int x, y;
            unsigned int width, height;
        };

        class Node {
            Space *const _space;

            node::Branch *_parent;

            std::list<Node *>::iterator _parent_iter;
            Attribute _attribute;

            explicit Node(Space *const &);

            virtual ~Node() = 0;

            virtual void _configure(const Attribute &);

            virtual void _refresh()=0;

            virtual node::Leaf *_activeLeaf(const FB &)=0;

            virtual Node *_activeChild()=0;

            virtual node::Branch *_receive(Node *const &, const FB &)=0;

            friend Space;
            friend node::Leaf;
            friend node::Branch;
        };

        namespace node {
            class Leaf : public Node {
                const Window _window;
                const std::unordered_map<Window, Leaf *>::iterator _leaves_iter;

                Leaf(Space *const &, const Window &);

                ~Leaf() final;

                void _refresh() final;

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

                void _configure(const Attribute &) final;

                void _refresh() final;

                node::Leaf *_activeLeaf(const FB &) final;

                Node *_activeChild() final;

                node::Branch *_receive(Node *const &, const FB &) final;

                void _configureChildren();

                friend Space;
                friend Leaf;
            };
        }

        const long root_event_mask = SubstructureNotifyMask;
        const long leaf_event_mask = FocusChangeMask | EnterWindowMask;

        auto matrix = [&](Display *const &display, const auto &breakLoop, const auto &callback) {
            auto space = Space(display, breakLoop);
            space.refresh();
            server::CommandHandlers command_handlers = {
                    {"exit",            [&]() {
                        space.exit();
                    }},

                    {"focus-up",        [&]() {
                        space.focus(HV::VERTICAL, FB::BACKWARD);
                    }},
                    {"focus-right",     [&]() {
                        space.focus(HV::HORIZONTAL, FB::FORWARD);
                    }},
                    {"focus-down",      [&]() {
                        space.focus(HV::VERTICAL, FB::FORWARD);
                    }},
                    {"focus-left",      [&]() {
                        space.focus(HV::HORIZONTAL, FB::BACKWARD);
                    }},

                    {"move-up",         [&]() {
                        space.move(HV::VERTICAL, FB::BACKWARD);
                    }},
                    {"move-right",      [&]() {
                        space.move(HV::HORIZONTAL, FB::FORWARD);
                    }},
                    {"move-down",       [&]() {
                        space.move(HV::VERTICAL, FB::FORWARD);
                    }},
                    {"move-left",       [&]() {
                        space.move(HV::HORIZONTAL, FB::BACKWARD);
                    }},

                    {"reparent-up",     [&]() {
                        space.reparent(HV::VERTICAL, FB::BACKWARD);
                    }},
                    {"reparent-right",  [&]() {
                        space.reparent(HV::HORIZONTAL, FB::FORWARD);
                    }},
                    {"reparent-down",   [&]() {
                        space.reparent(HV::VERTICAL, FB::FORWARD);
                    }},
                    {"reparent-left",   [&]() {
                        space.reparent(HV::HORIZONTAL, FB::BACKWARD);
                    }},

                    {"view-in",       [&]() {
                        space.viewResize(FB::FORWARD);
                    }},
                    {"view-out",      [&]() {
                        space.viewResize(FB::BACKWARD);
                    }},
                    {"view-forward",  [&]() {
                        space.viewMove(FB::FORWARD);
                    }},
                    {"view-backward", [&]() {
                        space.viewMove(FB::BACKWARD);
                    }},

                    {"transpose",       [&]() {
                        space.transpose();
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
}