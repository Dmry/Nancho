# Nancho

Uses code adapted from:

    dbus-print-message by Philip Blundell (GPL)
    pulsetest by Jason White (Public Domain)
    dbus-sample by MakerCrew on GitHub (MIT license)

Compilation dependencies:

    sudo apt install libdbus-1-dev libpulse-dev libboost-program-options-dev

Allowed arguments:
```
    -h [ --help ]                  Produce this help message.  
    -c [ --cooldown ] arg (=0)     Time in minutes after which the music will no longer resume. 0 disables cooldown.  
    -t [ --triggers ] arg          Names of the binaries that trigger a switch.  
    -p [ --player ] arg (=spotify) Name of the binary that is controlled by the switch.
```
