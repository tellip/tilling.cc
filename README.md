## Description

*matwm*(short for *Matrix Window Manager*) is a tilling window manager that represents windows as the leaves of a pure general, customizable tree.

It only responds to X events, and the messages it receives on a dedicated socket.

*matc* is a program that writes messages on *matwm*'s socket.

*matwm* doesn't handle any keyboard inputs: a third party program (e.g. *sxhkd*) is needed in order to translate keyboard events to *matc* invocations.

The outlined architecture is the following:

```
        PROCESS          SOCKET
sxhkd  -------->  matc  <------>  matwm
```

## Usage

```
cd [path_to_repo]
./build.sh
ln -s [path_to_repo]/cmake-build/matwm [path_to_binary]
ln -s [path_to_repo]/cmake-build/matc [path_to_binary]
nano ~/.xinitrc # add "exec matwm"
nano ~/.config/sxhkdrc # refer to sxhkdrc.sample
nano ~/.config/matwm/bashrc # refer to bashrc.sample
nano ~/.config/matwm/bash_logout # refer to bash_logout.sample
startx
```
