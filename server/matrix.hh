#pragma once

#include <X11/Xlib.h>
#include "main.hh"

namespace wm {
    namespace matrix {
        enum HV {
            HORIZONTAL = true, VERTICAL = false
        };

        enum FB {
            FORWARD = true, BACKWARD = false
        };

        class Space {
            Display *const _display;
            const unsigned long _normal_pixel, _focus_pixel;
            const Atom _xia_protocols, _xia_delete_window;

            unsigned int _display_width, _display_height;
            HV _display_hv;

            Node *_root, *_view;
            node::Leaf *_focus;
            std::unordered_map<Window, node::Leaf *> _leaves;

            unsigned long _colorPixel(const char *const &);

        public:
            const server::CommandHandlers command_handlers;
            const server::EventHandlers event_handlers;

            Space(Display *const &, const std::function<void()> &);

            void refresh();

            void focus(Node *const &);

            node::Branch *join(Node *const &, Node *const &, const FB &);

            friend Node;
            friend node::Branch;
            friend node::Leaf;
        };

        class Node {
        protected:
            Space *const _space;

            node::Branch *_parent;

            std::list<Node *>::iterator _iter_parent;
            HV _hv;
            int _x, _y;
            unsigned int _width, _height;

        public:
            explicit Node(Space *const &);

            virtual ~Node() = 0;

            virtual void configure(const HV &, const int &, const int &, const unsigned int &, const unsigned int &);

            virtual void refresh()=0;

            virtual node::Leaf *getActiveLeaf()=0;

        protected:
            virtual node::Branch *_receive(Node *const &, const FB &)=0;

            friend Space;
            friend node::Leaf;
            friend node::Branch;
        };

        namespace node {
            class Leaf : public Node {
                const Window _window;
                const std::unordered_map<Window, Leaf *>::iterator _iter_leaves;
            public:
                Leaf(Space *const &, const Window &);

                ~Leaf() final;

                void refresh() final;

                node::Leaf *getActiveLeaf() final;

            protected:
                node::Branch *_receive(Node *const &, const FB &) final;

                friend Space;
            };

            class Branch : public Node {
                std::list<Node *> _children;
                std::list<Node *>::iterator _active_iter;
            public:
                explicit Branch(Space *const &);

                void configure(const HV &, const int &, const int &, const unsigned int &, const unsigned int &) final;

                void refresh() final;

                node::Leaf *getActiveLeaf() final;

            protected:
                node::Branch *_receive(Node *const &, const FB &) final;

            public:

                void configureChildren();

                friend Space;
                friend Leaf;
            };
        }

        const long root_event_mask = SubstructureNotifyMask;
        const long leaf_event_mask = FocusChangeMask | EnterWindowMask;

        auto matrix = [&](Display *const &display, const auto &break_, const auto &callback) {
            auto space = Space(display, break_);
            space.refresh();

            callback(
                    space.command_handlers,
                    root_event_mask,
                    leaf_event_mask,
                    space.event_handlers
            );
        };
    }
}