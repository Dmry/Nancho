#ifndef SPOTIFY_H
#define SPOTIFY_H

#include "player.h"
#include "dbus_interface.h"
#include <dbus/dbus.h>

#include <string>

class Spotify : public Player
{
    public:
        Spotify();
        ~Spotify();

        void switch_state(const std::string&) override;
        Player::State fetch_status() override;

    private:
        DBus dbus;
};

#endif