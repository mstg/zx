# zx
i3 scratchpad manager

[ArchLinux thread](https://bbs.archlinux.org/viewtopic.php?pid=1611058)

[AUR package](https://aur.archlinux.org/packages/zx-git/)

```
usage: zx [ -h | -x | -H | -d | -b | -f | -n | -F | -a | -B | -p]
        -h shows help
        -x sets x offset
        -H sets height
        -d sets daemon mode
        -b sets background color
        -f sets font
        -n sets font color
        -F sets floating
        -a sets border color
        -B sets border
        -p pin to bottom of screen
```

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
font=fixed
pin_bottom=1
```

Config explanation
```
background (unsigned long / hex) - background color (ex. 0x000000)
border_color (unsigned long / hex) - border color (border around each window entry) (ex. 0xFFFFFF)
border (int) - enable/disable border
floating (int) - should windows opened from zx float or not
font_color (unsigned long / hex) - font color (ex. 0xFFFFFF)
daemon (int) - run in daemon mode or not
font (char) - font name
pin_bottom (int) - pin to bottom of screen
```

You can also show/hide zx on command by sending it a USR1 signal (ex. `killall -USR1 zx`)

You can also configure zx using cli arguments.

# Multimonitor
Just spawn another zx instance and pad x or y offset using `-x or -y` parameters

Example:

Two 1920x1080 monitors
```
zx
zx -x 1920
```

I also recommend setting `focus_follows_mouse` to `yes` in i3 config when using zx with a multimonitor setup
