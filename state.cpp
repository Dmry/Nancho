#include "state.h"

#include <iostream>

Machine::Machine(std::shared_ptr<Player> player)
    : m_player{player}, m_triggered{false}
{
    switch(m_player->fetch_status())
    {
        case Player::State::PLAY:
            current = new Playing();
            break;
        case Player::State::PAUSE:
            current = new Paused();
            break;
        default:
            current = new Paused();
            break;
    }
}

void Machine::set_current(State *s)
{
    current = s;
}

void Machine::play()
{
    current->play(this);
}

void Machine::pause()
{
    current->pause(this);
}

// Default response, only classes that change the state will overwrite
void State::play(Machine *m)
{
    std::cout << "already playing" << std::endl;
}

// Default response, only classes that change the state will overwrite
void State::pause(Machine *m)
{
    std::cout << "already OFF" << std::endl;
}

Playing::Playing()
{
}

Playing::~Playing()
{
}

Paused::Paused()
{
}

Paused::~Paused()
{
}

//Change state from paused to playing
void Paused::play(Machine *m)
{
    if (m->m_triggered) {
        std::cout << "going from paused to playing" << std::endl;
        m->set_current(new Playing());
        m->m_player->switch_state(Player::State::PLAY);
        delete this;

        m->m_triggered = false;
    }
}

//Change state from playing to paused
void Playing::pause(Machine *m)
{
    std::cout << "going from playing to paused" << std::endl;
    m->set_current(new Paused());
    m->m_player->switch_state(Player::State::PAUSE);
    delete this;

    m->m_triggered = true;
}