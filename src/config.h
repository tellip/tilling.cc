#pragma once

#include "wm.h"

namespace wm::config {
    const auto socket_host = "127.0.0.1";
    const in_port_t socket_port_base = 2056;

    const auto config_path_evn = "TREE_WM_CONFIG_PATH";

    const std::unordered_map<std::string, std::function<void(tree::Space & )>> command_handlers = {
            {"exit",             [](tree::Space &space) {
                space.exit();
            }},

            {"refresh",          [](tree::Space &space) {
                space.refresh(true);
            }},

            {"focus-up",         [](tree::Space &space) {
                space.focus(tree::HV::VERTICAL, tree::FB::BACKWARD);
            }},
            {"focus-right",      [](tree::Space &space) {
                space.focus(tree::HV::HORIZONTAL, tree::FB::FORWARD);
            }},
            {"focus-down",       [](tree::Space &space) {
                space.focus(tree::HV::VERTICAL, tree::FB::FORWARD);
            }},
            {"focus-left",       [](tree::Space &space) {
                space.focus(tree::HV::HORIZONTAL, tree::FB::BACKWARD);
            }},

            {"reorder-up",       [](tree::Space &space) {
                space.reorder(tree::HV::VERTICAL, tree::FB::BACKWARD);
            }},
            {"reorder-right",    [](tree::Space &space) {
                space.reorder(tree::HV::HORIZONTAL, tree::FB::FORWARD);
            }},
            {"reorder-down",     [](tree::Space &space) {
                space.reorder(tree::HV::VERTICAL, tree::FB::FORWARD);
            }},
            {"reorder-left",     [](tree::Space &space) {
                space.reorder(tree::HV::HORIZONTAL, tree::FB::BACKWARD);
            }},

            {"reparent-up",      [](tree::Space &space) {
                space.reparent(tree::HV::VERTICAL, tree::FB::BACKWARD);
            }},
            {"reparent-right",   [](tree::Space &space) {
                space.reparent(tree::HV::HORIZONTAL, tree::FB::FORWARD);
            }},
            {"reparent-down",    [](tree::Space &space) {
                space.reparent(tree::HV::VERTICAL, tree::FB::FORWARD);
            }},
            {"reparent-left",    [](tree::Space &space) {
                space.reparent(tree::HV::HORIZONTAL, tree::FB::BACKWARD);
            }},

            {"reorganize-up",    [](tree::Space &space) {
                space.reorganize(tree::HV::VERTICAL, tree::FB::BACKWARD);
            }},
            {"reorganize-right", [](tree::Space &space) {
                space.reorganize(tree::HV::HORIZONTAL, tree::FB::FORWARD);
            }},
            {"reorganize-down",  [](tree::Space &space) {
                space.reorganize(tree::HV::VERTICAL, tree::FB::FORWARD);
            }},
            {"reorganize-left",  [](tree::Space &space) {
                space.reorganize(tree::HV::HORIZONTAL, tree::FB::BACKWARD);
            }},

            {"view-in",          [](tree::Space &space) {
                space.viewResize(tree::FB::FORWARD);
            }},
            {"view-out",         [](tree::Space &space) {
                space.viewResize(tree::FB::BACKWARD);
            }},
            {"view-leaf",        [](tree::Space &space) {
                space.viewExtreme(tree::FB::FORWARD);
            }},
            {"view-root",        [](tree::Space &space) {
                space.viewExtreme(tree::FB::BACKWARD);
            }},
            {"view-forward",     [](tree::Space &space) {
                space.viewMove(tree::FB::FORWARD);
            }},
            {"view-backward",    [](tree::Space &space) {
                space.viewMove(tree::FB::BACKWARD);
            }},

            {"transpose",        [](tree::Space &space) {
                space.transpose();
            }},

            {"close-window",     [](tree::Space &space) {
                space.closeActive(false);
            }},
            {"kill-window",      [](tree::Space &space) {
                space.closeActive(true);
            }}
    };
}