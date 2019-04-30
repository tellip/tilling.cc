## Description

*tree_wm* (short for *Matrix Window Manager*) is a tilling window manager that represents windows as the leaves of a pure general, customizable tree.

It only responds to X events, and the messages it receives on a dedicated socket.

*tree_wm client* writes messages on *tree_wm server*'s socket.

*tree_wm server* doesn't handle any keyboard inputs: a third party program (e.g. *sxhkd*) is needed in order to translate keyboard events to *tree_wm client* invocations.

The outlined architecture is the following:

```
        PROCESS                    SOCKET
sxhkd  -------->  tree_wm client  <------>  tree_wm server
```

## Usage

```
cd [path_to_repo]

cmake .
make
cp tree_wm ~/bin # make sure '~/bin' is in environment variable 'PATH'

nano ~/.config/sxhkdrc              # refer to      *sample/sxhkdrc*
nano ~/.config/tree_wm/tree_wm_rc    # refer to      *sample/tree_wm_rc*
nano ~/.xinitrc                     # add           '~/.config/tree_wm/tree_wm_rc &' and 'exec tree_wm server'
startx
```
