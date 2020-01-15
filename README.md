## Description

*tilling* (short for *Tilling Desktop*) is a tilling desktop environment that represents windows as the leaves of a pure general, customizable, tilling tree.

It only responds to X events, and the messages it receives on a dedicated socket.

*tilling client* writes messages on *tilling server*'s socket.

*tilling server* doesn't handle any keyboard inputs: a third party program (e.g. *sxhkd*) is needed in order to translate keyboard events to *tilling client* invocations.

The outlined architecture is the following:

```
        PROCESS                    SOCKET
sxhkd  -------->  tilling client  <------>  tilling server
```

## Usage

```
cd [path_to_repo]

cmake .
make
cp tilling ~/bin # make sure '~/bin' is in environment variable 'PATH'

nano ~/.config/sxhkd/sxhkdrc	# refer to      *sample/sxhkdrc*
nano ~/.config/tilling.json	# refer to      *sample/config.json*
nano ~/.xinitrc			# add
    # export TILLING_CONFIG_PATH="${HOME}/.config/tilling.json"
    # sxhkd &
    # exec tilling server
startx
```
