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
		};
		HV display_hv = HV::HORIZONTAL;

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

		std::function<void(Node *const &, const HV &, const int &, const int &, const unsigned int &, const unsigned int &)> configureNode;

		auto configureChildren = [&](Node *const &node) {
			if (node->node_type == Node::Type::Branch) {
				auto &children = node->derived.branch->children;
				switch (node->hv) {
					case HV::HORIZONTAL: {
						auto s = node->x + config::border_width, d = (node->width - config::border_width * 2) / children.size();
						for (auto i = children.cbegin(); i != std::prev(children.cend()); i++) {
							configureNode(*i, (HV) !node->hv, s, node->y + config::border_width, d, node->height - config::border_width * 2);
							s += d;
						}
						configureNode(*(std::prev(children.cend())), (HV) !node->hv, s, node->y + config::border_width, node->x + node->width - config::border_width - s, node->height - config::border_width * 2);
						break;
					}
				}
			}
		};

		configureNode = [&](Node *const &node, const HV &hv, const int &x, const int &y, const unsigned int &width, const unsigned int &height) {
			node->hv = hv;
			node->x = x;
			node->y = y;
			node->width = width;
			node->height = height;
			if (node->node_type == Node::Type::Branch) configureChildren(node);
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

		auto destructLeaf = [&](Node *const &node) -> bool {
			if (node && node->node_type == Node::Type::Leaf && !node->parent) {
				nodes.erase(node->derived.leaf->key);
				delete node;
				return true;
			} else return false;
		};

		auto constructBranch = [&]() -> Node * {
			return new Node(Node::Type::Branch, ({
				typename Node::Derived d;
				d.branch = new typename Node::Branch();
				d;
			}));
		};

		auto nodeJoin = [&](Node *const &node, Node *const &target, const FB &fb) {
			if (node && !node->parent && target) {
				target->poly(
						{
								{Node::Type::Leaf,   [&]() {
									auto new_grand_parent = target->parent;
									auto new_parent = constructBranch();
									if (new_grand_parent) {
										auto &grand_children = new_grand_parent->derived.branch->children;
										new_parent->position = grand_children.insert(target->position, new_parent);
										new_parent->parent = new_grand_parent;
										grand_children.erase(target->position);
									} else new_parent->parent = NULL;
									auto &children = new_parent->derived.branch->children;
									target->position = children.insert(children.end(), target);
									target->parent = new_parent;
									node->position = children.insert(fb ? std::next(target->position) : target->position, node);
									node->parent = new_parent;
								}},
								{Node::Type::Branch, [&]() {
									auto &children = target->derived.branch->children;
									node->position = children.insert(fb ? children.end() : children.begin(), node);
									node->parent = target;
								}}
						}
				);
			}
		};

		auto nodeMove = [&](Node *const &node, const typename std::list<Node *>::iterator &position, const FB &fb) {
			if (node && node->parent) {
				auto &children = node->parent->derived.branch->children;
				children.erase(node->position);
				node->position = children.insert(fb ? std::next(position) : position, node);
			}
		};

		auto nodeQuit = [&](Node *const &node) {
			if (node) {
				auto &parent = node->parent;
				if (parent) {
					auto &children = parent->derived.branch->children;
					children.erase(node->position);
					node->parent = NULL;
					if (children.size() == 1) {
						auto &child = *children.cbegin();
						children.erase(child->position);
						auto &grand_parent = parent->parent;
						if (grand_parent) {
							auto &grand_children = grand_parent->derived.branch->children;
							child->position = grand_children.insert(parent->position, child);
							child->parent = grand_parent;
							grand_children.erase(parent->position);
						} else child->parent = NULL;
					}
				}
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
								   XMoveResizeWindow(display, node->derived.leaf->window, node->x, node->y, node->width - config::border_width * 2, node->height - config::border_width * 2);
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
				configureNode(view, display_hv, -config::border_width, -config::border_width, display_width + config::border_width * 2, display_height + config::border_width * 2);
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
								{MapNotify,   [&](const XEvent &event) {
									auto xmap = event.xmap;
									auto window = xmap.window;
									if (nodes.find(window) == nodes.end() && !xmap.override_redirect) {
										auto node = constructLeaf(window);
										node->parent = NULL;

										if (!active) {
											configureNode(node, display_hv, -config::border_width, -config::border_width, display_width + config::border_width * 2, display_height + config::border_width * 2);

											refreshNode(node);

											root = view = node;
										} else {
											refreshLeafFocus(active, false);

											if (active == view) {
												nodeJoin(node, active, FB::FORWARD);

												configureNode(node->parent, display_hv, -config::border_width, -config::border_width, display_width + config::border_width * 2, display_height + config::border_width * 2);

												refreshNode(node->parent);

												view = node->parent;
											} else {
												nodeJoin(node, active->parent, FB::FORWARD);
												nodeMove(node, active->position, FB::FORWARD);

												configureChildren(node->parent);

												refreshNode(node->parent);
											}
										}

										focusNode(node);

										refreshLeafFocus(node, true);

										active = node;
									}
								}},
								{UnmapNotify, [&](const XEvent &event) {
									auto window = event.xunmap.window;
									auto i = nodes.find(window);
									if (i != nodes.end()) {
										auto node = i->second;
										auto parent = node->parent;
										if (parent) {
											auto &children = parent->derived.branch->children;
											if (children.size() > 2) {
//												auto new_active = (active == node ? *(node->position == children.begin() ? std::next(node->position) : std::prev(node->position)) : NULL);

												nodeQuit(node);
												destructLeaf(node);

												configureChildren(parent);

												refreshNode(parent);

//												if (new_active) {
//													focusNode(new_active);
//
//													refreshLeafFocus(new_active, true);
//
//													active
//												}
											} else {
												//...
											}
										} else {
											//...
										}
									}
								}}
						}
				)
		);
	};
}