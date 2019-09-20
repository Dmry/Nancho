#include "spotify.h"

#include <algorithm>

//remove:
#include <iostream>

Spotify::Spotify()
    : Player(), dbus{DBus()}
{
    command_map[State::PLAY] = "Play";
    command_map[State::PAUSE] = "Pause";
}

Spotify::~Spotify()
{
}

void Spotify::switch_state(Player::State state)
{
    DBusMessage * dbus_reply = nullptr;
    
    DBusMessage * dbus_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.spotify", "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", Player::command_map[state].c_str());

    dbus.send(dbus_msg, &dbus_reply);
}

Player::State Spotify::fetch_status()
{
    DBusMessage * dbus_reply = nullptr;
    DBusMessage * dbus_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.spotify", "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");

    const char *player = "org.mpris.MediaPlayer2.Player";
    const char *status = "PlaybackStatus";

    dbus_message_append_args (dbus_msg, DBUS_TYPE_STRING, &player, DBUS_TYPE_STRING, &status, DBUS_TYPE_INVALID);

    dbus.send(dbus_msg, &dbus_reply);

    if (dbus_reply)
    {
        std::string reply = dbus.get_string_reply(dbus_reply);

        if (reply == "Playing")
        {
            return Player::State::PLAY;
        }
        else if (reply == "Paused")
        {
            return Player::State::PAUSE;
        }
    }
    
    //else
    return State::UNKNOWN;
}