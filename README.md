# zx
i3 scratchpad manager

# Status
This is fully working, very little tested.

# Requirements
* i3ipc-glib
* glib
* libxcb

# Installation
Install the requirements then run `make` then `sudo make install`

You can run this from your i3 config to run on startup

# Config
You can configure zx with a config file in ~/.zxconfig

Example config file:
```
[zx]
background=0x2C3E50
border_color=0x7F8C8D
border=1
floating=0
font_color=0xFFFFFF
```
