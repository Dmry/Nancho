# Nancho

Automatically pause MPRIS music players (e.g. spotify) when another application (e.g. your browser) starts playing sound.

### Compilation dependencies

Ubuntu

    sudo apt install libdbus-1-dev libpulse-dev meson

Arch linux

    sudo pacman -S dbus pulseaudio meson

### Compile and install

```
mkdir build
meson setup build
meson install
```

Then as normal user

```
systemctl --user enable nancho
```

### Running from commandline

```
Automatically pause music player when a trigger starts playing sound
Usage:
  Nancho [OPTION...]

  -c, --cooldown arg  Time in minutes after which the music will no longer resume. 0 disables cooldown (default: 0)
  -d, --delay arg     Time in seconds to wait before pausing music (default: 0)
      --trigger arg   Names of the binaries that trigger a switch (default: firefox)
  -p, --player arg    Name of the binary that is controlled by the switch.(default: spotify)
      --debug         Enable debugging
  -h, --help          Print this help message
```

### Acknowledgements

Uses code adapted from:

    dbus-print-message by Philip Blundell (GPL)
    pulsetest by Jason White (Public Domain)
    dbus-sample by MakerCrew on GitHub (MIT license)