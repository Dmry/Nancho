#include "nancho.h"

#include <cstddef>
#include <cstdio>
#include <iostream>

int main() {

    DBusError dbus_error;
    DBusConnection * dbus_conn = nullptr;

    // Initialize D-Bus error
    ::dbus_error_init(&dbus_error);

    // Connect to D-Bus
    if ( nullptr == (dbus_conn = ::dbus_bus_get(DBUS_BUS_SESSION, &dbus_error)) ) {
        ::perror(dbus_error.name);
        ::perror(dbus_error.message);
    }

    pause_spotify(dbus_conn, dbus_error);


    ::dbus_connection_unref(dbus_conn); 

    return 0;
}


int pause_spotify(DBusConnection * dbus_conn_, DBusError dbus_error_) {
    DBusMessage * dbus_msg = nullptr;
    DBusMessage * dbus_reply = nullptr;

    // Compose remote procedure call
    if ( nullptr == (dbus_msg = ::dbus_message_new_method_call("org.mpris.MediaPlayer2.spotify", "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "PlayPause")) ) {
        ::dbus_connection_unref(dbus_conn_);
        ::perror("ERROR: ::dbus_message_new_method_call - Unable to allocate memory for the message!");

    // Invoke remote procedure call, block for response
    } else if ( nullptr == (dbus_reply = ::dbus_connection_send_with_reply_and_block(dbus_conn_, dbus_msg, DBUS_TIMEOUT_USE_DEFAULT, &dbus_error_)) ) {
        ::dbus_message_unref(dbus_msg);
        ::dbus_connection_unref(dbus_conn_);
        ::perror(dbus_error_.name);
        ::perror(dbus_error_.message);
    }
}