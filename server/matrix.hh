#pragma once

#include <X11/Xlib.h>
#include "main.hh"

namespace wm {
    namespace matrix {
        class PointerCoordinate {
            Display *const _display;

            int _x, _y;

            Window _root, _child;
            int _root_x, _root_y, _win_x, _win_y;
            unsigned int _mask;

            explicit PointerCoordinate(Display *const &);

            void _refresh();

            void _record();

            bool _check();

            friend Space;
        };

        class Space {
            const std::function<void()> _breakLoop;
            Display *const _display;
            const unsigned long _normal_pixel, _focus_pixel;
            const Atom _xia_protocols, _xia_delete_window;

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

            void _focus(Node *const &, const bool &);

        public:
            void refresh(const bool & = false);

            void exit();

            void focus(const HV &, const FB &);

            void move(const HV &, const FB &);

            void join(const HV &, const FB &);

            void quit(const HV &, const FB &);

            void transpose();

            friend Node;
            friend node::Branch;
            friend node::Leaf;
        };

        class Node {
            Space *const _space;

            node::Branch *_parent;

            std::list<Node *>::iterator _parent_iter;
            HV _hv;
            int _x, _y;
            unsigned int _width, _height;

            explicit Node(Space *const &);

            virtual ~Node() = 0;

            virtual void _configure(const HV &, const int &, const int &, const unsigned int &, const unsigned int &);

            virtual void _refresh()=0;

            virtual node::Leaf *_getActiveLeaf()=0;

            virtual node::Leaf *_setActiveLeaf(const FB &)=0;

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

                node::Leaf *_getActiveLeaf() final;

                node::Leaf *_setActiveLeaf(const FB &) final;

                node::Branch *_receive(Node *const &, const FB &) final;

                friend Space;
            };

            class Branch : public Node {
                std::list<Node *> _children;
                std::list<Node *>::iterator _iter_active;

                explicit Branch(Space *const &);

                void _configure(const HV &, const int &, const int &, const unsigned int &, const unsigned int &) final;

                void _refresh() final;

                node::Leaf *_getActiveLeaf() final;

                node::Leaf *_setActiveLeaf(const FB &) final;

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
            space.refresh(true);
            server::CommandHandlers command_handlers = {
                    {"exit",        [&]() {
                        space.exit();
                    }},

                    {"focus-up",    [&]() {
                        space.focus(HV::VERTICAL, FB::BACKWARD);
                    }},
                    {"focus-right", [&]() {
                        space.focus(HV::HORIZONTAL, FB::FORWARD);
                    }},
                    {"focus-down",  [&]() {
                        space.focus(HV::VERTICAL, FB::FORWARD);
                    }},
                    {"focus-left",  [&]() {
                        space.focus(HV::HORIZONTAL, FB::BACKWARD);
                    }},

                    {"move-up",     [&]() {
                        space.move(HV::VERTICAL, FB::BACKWARD);
                    }},
                    {"move-right",  [&]() {
                        space.move(HV::HORIZONTAL, FB::FORWARD);
                    }},
                    {"move-down",   [&]() {
                        space.move(HV::VERTICAL, FB::FORWARD);
                    }},
                    {"move-left",   [&]() {
                        space.move(HV::HORIZONTAL, FB::BACKWARD);
                    }},

                    {"join-up",     [&]() {
                        space.join(HV::VERTICAL, FB::BACKWARD);
                    }},
                    {"join-right",  [&]() {
                        space.join(HV::HORIZONTAL, FB::FORWARD);
                    }},
                    {"join-down",   [&]() {
                        space.join(HV::VERTICAL, FB::FORWARD);
                    }},
                    {"join-left",   [&]() {
                        space.join(HV::HORIZONTAL, FB::BACKWARD);
                    }},

                    {"quit-up",     [&]() {
                        space.quit(HV::VERTICAL, FB::BACKWARD);
                    }},
                    {"quit-right",  [&]() {
                        space.quit(HV::HORIZONTAL, FB::FORWARD);
                    }},
                    {"quit-down",   [&]() {
                        space.quit(HV::VERTICAL, FB::FORWARD);
                    }},
                    {"quit-left",   [&]() {
                        space.quit(HV::HORIZONTAL, FB::BACKWARD);
                    }},

                    {"transpose",   [&]() {
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