[![Build Status](https://travis-ci.org/Dmry/Nancho.svg?branch=master)](https://travis-ci.org/Dmry/Nancho) [![CodeFactor](https://www.codefactor.io/repository/github/dmry/nancho/badge)](https://www.codefactor.io/repository/github/dmry/nancho) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# Nancho

Automatically pause MPRIS music players (e.g. spotify) when another application (e.g. your browser) starts playing sound.

### Compilation dependencies

Ubuntu

    sudo apt install libdbus-1-dev libpulse-dev

Arch linux

    sudo pacman -S dbus pulseaudio

### Compile and install

From the nancho root folder, as root

    mkdir build
    cd build
    cmake ..
    make install
    cp ../nancho.service /etc/systemd/user/nancho.service

Then as normal user

    systemctl --user enable nancho

### Running from commandline

Allowed arguments
```
 -h, --help                     Show help  
 -c, --cooldown [integer]       Time in minutes after which the music will no longer resume. 0 disables cooldown.  
 -d, --delay [integer]          Time in seconds to wait before pausing and resuming music.  
 -t, --trigger [string...]      Names of the binaries that trigger a switch.  
 -p, --player [string]          Name of the binary that is controlled by the switch.  
```

### Acknowledgements

Uses code adapted from:

    dbus-print-message by Philip Blundell (GPL)
    pulsetest by Jason White (Public Domain)
    dbus-sample by MakerCrew on GitHub (MIT license)
