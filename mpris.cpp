#include "mpris.h"

#include <algorithm>

//remove:
#include <iostream>

Mpris::Mpris(const std::string& player)
    : Player("org.mpris.MediaPlayer2."+player), dbus{DBus()}
{
}

Mpris::~Mpris()
{
}

void Mpris::switch_state(const std::string& command)
{
    DBusMessage * dbus_reply = nullptr;
    
    DBusMessage * dbus_msg = dbus_message_new_method_call(m_player.c_str(), "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", command.c_str());

    dbus.send(dbus_msg, &dbus_reply);
}

Player::State Mpris::fetch_status()
{
    DBusMessage * dbus_reply = nullptr;
    DBusMessage * dbus_msg = dbus_message_new_method_call(m_player.c_str(), "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");

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