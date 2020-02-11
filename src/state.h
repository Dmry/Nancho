#ifndef STATE_H
#define STATE_H

#include "mpris.h"

#include <memory>
#include <chrono>
#include <vector>
#include <future>
#include <thread>

class State;

class Machine
{
    private:
        class std::shared_ptr<State> current;
        std::vector<std::future<void>> pending_futures;

    public:
        Machine(std::shared_ptr<Player> player,
                std::chrono::minutes cooldown = std::chrono::minutes(0),
                std::chrono::seconds delay = std::chrono::seconds(0));

        void set_current(std::shared_ptr<State> s);
        void play();
        void pause();
        void fetch();

        std::shared_ptr<Player> m_player;   
        std::chrono::minutes m_cooldown;
        std::chrono::seconds m_delay;
};

class State
{
    public:
        virtual void play(Machine *m);
        virtual void pause(Machine *m);

        Player::State m_current_state;
        Player::State m_previous_state;

        std::chrono::steady_clock::time_point m_time_switched;
};

class Playing: public State
{
    public:
        Playing(Player::State);
        ~Playing();
        void pause(Machine *m) override;
        // Does not override play, keeps its default behavior
};

class Paused: public State
{
    public:
        Paused(Player::State);
        ~Paused();
        void play(Machine *m) override;
        // Does not override pause, keeps its default behavior
};

#endif