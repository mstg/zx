# zx
i3 scratchpad manager

[ArchLinux thread](https://bbs.archlinux.org/viewtopic.php?pid=1611058)

[AUR package](https://aur.archlinux.org/packages/zx-git/)

# About zx
zx creates a taskbar like window on the bottom of the screen to show you windows which have been minimized (sent to scratchpad). You can easily reopen them in any workspace you like.

Default:
![Default](https://raw.githubusercontent.com/mstg/zx/master/screenshots/default.png)

No-border:
~/.zxconfig:
```
[zx]
border=0
```
![No-border](https://raw.githubusercontent.com/mstg/zx/master/screenshots/no-border.png)

# Status
This is fully working, very little tested.

# Requirements
* i3ipc-glib
* glib
* libxcb

# Installation
Install the requirements then run `make` then `sudo make install`

You can run this from your i3 config to run on startup

# Installation (Arch AUR)
`yaourt -S zx-git`


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
daemon=0
```

Config explanation
```
background (unsigned long / hex) - background color (ex. 0x000000)
border_color (unsigned long / hex) - border color (border around each window entry) (ex. 0xFFFFFF)
border (int) - enable/disable border
floating (int) - should windows opened from zx float or not
font_color (unsigned long / hex) - font color (ex. 0xFFFFFF)
daemon (int) - run in daemon mode or not
```

You can also show/hide zx on command by sending it a USR1 signal (ex. `killall -USR1 zx`)
