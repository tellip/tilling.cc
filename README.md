## Description

*tree_wm* (short for *Tree Window Manager*) is a tilling window manager that represents windows as the leaves of a pure general, customizable tree.

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

nano ~/.config/sxhkd/sxhkdrc        # refer to      *sample/sxhkdrc*
mkdir -p ~/.config/tree_wm
nano ~/.config/tree_wm/rc           # refer to      *sample/rc*
nano ~/.config/tree_wm/config.json  # refer to      *sample/config.json*
nano ~/.xinitrc                     # add
    # export TREE_WM_CONFIG_PATH="${HOME}/.config/tree_wm/config.json"
    # ~/.config/tree_wm/rc &
    # exec tree_wm server
startx
```
