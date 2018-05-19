#pragma once

#include <X11/Xlib.h>
#include "main.hh"

namespace matrix_wm {
	auto control = [&](const std::function<void()> &breakListen, Display *const &display, const std::function<void()> &breakLoop, const auto &callback) {
		unsigned long normal_pixel, focused_pixel;
		[&](const auto &cp) {
			normal_pixel = cp(config::normal_color);
			focused_pixel = cp(config::focused_color);
		}([&](const char *const &cc) {
			auto cm = DefaultColormap(display, XDefaultScreen(display));
			XColor x_color;
			if (XAllocNamedColor(display, cm, cc, &x_color, &x_color) == 0) error("XAllocNamedColor");
			return x_color.pixel;
		});

		auto xia_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
		auto xia_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
		if (xia_protocols == None || xia_delete_window == None) error("XInternAtom");

		int display_width, display_height;

		enum HV {
			HORIZONTAL = true, VERTICAL = false
		} display_hv;

		enum FB {
			FORWARD = true, BACKWARD = false
		};

		struct Node {
			Node *parent;
			typename std::list<Node *>::iterator position;
			HV hv;
			int x, y;
			unsigned int width, height;

			enum Type {
				Leaf, Branch
			};
			const Type node_type;

			struct Leaf {
				const Window window;
				const typename std::unordered_map<Window, Node *>::iterator key;

				Leaf(const Window &w, const typeof(key) &k) : window(w), key(k) {}
			};

			struct Branch {
				std::list<Node *> children;
				typename std::list<Node *>::iterator focused_position;
			};

			union Derived {
				typename Node::Leaf *leaf;
				typename Node::Branch *branch;
			};
			const Derived derived;

			Node(const Type &t, Derived &&d) : node_type(t), derived(std::move(d)) {}

			void poly(const std::unordered_map<int, std::function<void()>> &vf) {
				vf.find(node_type)->second();
			};
		};

		Node *root = NULL, *view = NULL, *active = NULL;
		std::unordered_map<Window, Node *> nodes;

		auto configureNode = [&](Node *const &node, const HV &hv, const int &x, const int &y, const unsigned int &width, const unsigned int &height) {
			node->hv = hv;
			node->x = x;
			node->y = y;
			node->width = width;
			node->height = height;
			node->poly({
							   {Node::Type::Leaf,   [&]() {}},
							   {Node::Type::Branch, [&]() {
								   //...
							   }}
					   });
		};

		auto constructLeaf = [&](const Window &window) -> Node * {
			auto i = nodes.insert(std::make_pair(window, (Node *) NULL)).first;
			auto node = i->second = new Node(Node::Type::Leaf, ({
				typename Node::Derived d;
				d.leaf = new typename Node::Leaf(window, i);
				d;
			}));
			XSetWindowBorderWidth(display, window, config::border_width);

			return node;
		};

		auto constructBranch = [&]() -> Node * {
			return new Node(Node::Type::Branch, ({
				typename Node::Derived d;
				d.branch = new typename Node::Branch();
				d;
			}));
		};

		auto joinNode = [&](Node *const &node, Node *const &target, const FB &fb) {
			auto old_parent = node->parent;
			if (old_parent != target) {
				if (old_parent) {
					//...
				}
				target->poly(
						{
								{Node::Type::Leaf,   [&]() {
									auto new_grand_parent = target->parent;
									auto new_parent = new Node(Node::Type::Branch, ({
										typename Node::Derived d;
										d.branch = new typename Node::Branch();
										d;
									}));
									if (new_grand_parent) {
										//...
									}
									new_parent->parent = new_grand_parent;
									auto &children = new_parent->derived.branch->children;
									target->position = children.insert(children.end(), target);
									target->parent = new_parent;
									node->position = children.insert(fb ? target->position : std::next(target->position), node);
									node->parent = new_parent;

									configureNode(new_parent, target->hv, target->x, target->y, target->width, target->height);
								}},
								{Node::Type::Branch, [&]() {
									//...
								}}
						}
				);
			}
		};

		std::function<void(Node *const &)> focusNode = [&](Node *const &node) {
			if (node->parent && node->parent->derived.branch->focused_position != node->position) {
				node->parent->derived.branch->focused_position = node->position;
				focusNode(node->parent);
			}
		};

		std::function<void(Node *const &)> refreshNode = [&](Node *const &node) {
			node->poly({
							   {Node::Type::Leaf,   [&]() {
								   XMoveResizeWindow(display, node->derived.leaf->window, node->x, node->y, node->width, node->height);
							   }},
							   {Node::Type::Branch, [&]() {
								   auto &children = node->derived.branch->children;
								   for (auto i = children.cbegin(); i != children.cend(); i++) {
									   refreshNode(*i);
								   }
							   }}
					   });
		};

		auto refreshLeafFocus = [&](Node *const &node, const bool &focused) {
			if (node->node_type == Node::Type::Leaf) {
				XSetWindowBorder(display, node->derived.leaf->window, focused ? focused_pixel : normal_pixel);
			}
		};

		auto refresh = [&]() {
			display_width = XDisplayWidth(display, DefaultScreen(display));
			display_height = XDisplayHeight(display, DefaultScreen(display));
			display_hv = (HV) (display_width >= display_height);
			if (view) {
				configureNode(view, display_hv, 0, 0, display_width, display_height);
				refreshNode(view);
			}
		};

		refresh();

		callback(
				//commands_handlers
				CommandHandlers(
						{
								{"exit", [&]() {
									breakListen();
									breakLoop();
								}}
						}
				),
				//event_masks
				SubstructureNotifyMask,
				//event_handlers
				EventHandlers(
						{
								{MapNotify, [&](const XEvent &event) {
									auto xmap = event.xmap;
									auto window = xmap.window;
									if (nodes.find(window) == nodes.end() && !xmap.override_redirect) {
										auto node = constructLeaf(window);
										node->parent = NULL;

										if (!view) {
											configureNode(node, display_hv, 0, 0, display_width - config::border_width * 2, display_height - config::border_width * 2);

											refreshNode(node);

											view = active = node;
										} else {
											view->poly(
													{
															{Node::Type::Leaf,   [&]() {
																refreshLeafFocus(active, false);

																joinNode(node, view, FB::BACKWARD);

																refreshNode(node->parent);

																view = node->parent;
																active = node;
															}},
															{Node::Type::Branch, [&]() {
																//...
															}}
													}
											);
										}

										focusNode(node);

										refreshLeafFocus(node, true);
									}
								}}
						}
				)
		);
	};
}