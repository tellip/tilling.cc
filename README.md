## Description

*matwm* is a tilling window manager that represents windows as the leaves of a pure tree.

It only responds to X events, and the messages it receives on a dedicated socket.

*bspc* is a program that writes messages on *bspwm*'s socket.

*bspwm* doesn't handle any keyboard inputs: a third party program (e.g. *sxhkd*) is needed in order to translate keyboard events to *bspc* invocations.
