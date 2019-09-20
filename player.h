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

        std::map<State, std::string> command_map;

        virtual void switch_state(Player::State state) = 0;
        virtual Player::State fetch_status() = 0;
};

#endif