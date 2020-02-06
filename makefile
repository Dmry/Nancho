all:
	g++ std=c++14 dbus_interface.cpp nancho.cpp player.cpp pulse.cpp mpris.cpp state.cpp trigger.cpp -I/usr/include/dbus-1.0/ -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -Wl,-Bstatic -lboost_program_options -Wl,-Bdynamic -ldbus-1 -lpthread -lpulse  -o nancho
