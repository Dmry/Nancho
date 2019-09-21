#ifndef STATE_H
#define STATE_H

#include "spotify.h"

#include <memory>

class State;

class Machine
{
    private:
        class std::shared_ptr<State> current;

    public:
        Machine(std::shared_ptr<Player> player);
        void set_current(std::shared_ptr<State> s);
        void play();
        void pause();
        void fetch();

        std::shared_ptr<Player> m_player;   
};

class State
{
    public:
        virtual void play(Machine *m);
        virtual void pause(Machine *m);

        Player::State m_state;
};

class Playing: public State
{
    public:
        Playing();
        ~Playing();
        void pause(Machine *m) override;
        // Does not override play, keeps its default behavior
};

class Paused: public State
{
    public:
        Paused(bool triggered = true);
        ~Paused();
        void play(Machine *m) override;
        // Does not override pause, keeps its default behavior

        bool m_triggered; // Tracks if nancho paused the music so it doesn't start playing inadvertedly
};

#endif