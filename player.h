#ifndef PLAYER_H
#define PLAYER_H

#include <map>

class Player
{
    public:
        Player();
        ~Player();

        enum class State {
            PLAY,
            PAUSE,
            UNKNOWN
        };

        virtual void switch_state(const std::string&) = 0;
        virtual Player::State fetch_status() = 0;
};

#endif