#ifndef PLAYER_H
#define PLAYER_H

#include <map>
#include <vector>
#include <string>

class Player
{
    public:
        Player(const std::string&);
        virtual ~Player();

        enum class State {
            PLAY,
            PAUSE,
            UNKNOWN
        };

        virtual void switch_state(const std::string&) = 0;
        virtual Player::State fetch_status() = 0;

        std::string m_player;
};

#endif