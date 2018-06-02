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
            node::Leaf *_active;
            std::unordered_map<Window, node::Leaf *> _leaves;

        public:
            const server::CommandHandlers command_handlers;
            const server::EventHandlers event_handlers;

            Space(Display *const &, const std::function<void()> &);

        private:
            unsigned long _colorPixel(const char *const &);

            void _refresh();

            node::Branch *_join(Node *const &, Node *const &, const FB &);

            void _move(Node *const &, const std::list<Node *>::iterator &);

            void _focus(Node *const &, const bool &);

            void _focus(const HV &, const FB &);

            friend Node;
            friend node::Branch;
            friend node::Leaf;
        };

        class Node {
            Space *const _space;

            node::Branch *_parent;

            std::list<Node *>::iterator _iter_parent;
            HV _hv;
            int _x, _y;
            unsigned int _width, _height;

            explicit Node(Space *const &);

            virtual ~Node() = 0;

            virtual void _configure(const HV &, const int &, const int &, const unsigned int &, const unsigned int &);

            virtual void _refresh()=0;

            virtual node::Leaf *_getActiveLeaf()=0;

            virtual node::Branch *_receive(Node *const &, const FB &)=0;

            friend Space;
            friend node::Leaf;
            friend node::Branch;
        };

        namespace node {
            class Leaf : public Node {
                const Window _window;
                const std::unordered_map<Window, Leaf *>::iterator _iter_leaves;

                Leaf(Space *const &, const Window &);

                ~Leaf() final;

                void _refresh() final;

                node::Leaf *_getActiveLeaf() final;

                node::Branch *_receive(Node *const &, const FB &) final;

                friend Space;
            };

            class Branch : public Node {
                std::list<Node *> _children;
                std::list<Node *>::iterator _active_iter;

                explicit Branch(Space *const &);

                void _configure(const HV &, const int &, const int &, const unsigned int &, const unsigned int &) final;

                void _refresh() final;

                node::Leaf *_getActiveLeaf() final;

                node::Branch *_receive(Node *const &, const FB &) final;

                void _configureChildren();

                friend Space;
                friend Leaf;
            };
        }

        const long root_event_mask = SubstructureNotifyMask;
        const long leaf_event_mask = FocusChangeMask | EnterWindowMask;

        auto matrix = [&](Display *const &display, const auto &break_, const auto &callback) {
            auto space = Space(display, break_);
            callback(
                    space.command_handlers,
                    root_event_mask,
                    leaf_event_mask,
                    space.event_handlers
            );
        };
    }
}