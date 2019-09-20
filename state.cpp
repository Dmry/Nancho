#include "state.h"

#include <iostream>

Machine::Machine(std::shared_ptr<Player> player)
    : m_player{player}, m_triggered{false}
{
    fetch();
}

void Machine::fetch()
{
    switch(m_player->fetch_status())
    {
        case Player::State::PLAY:
            current = std::make_shared<Playing>();
            break;
        case Player::State::PAUSE:
            current = std::make_shared<Paused>();
            break;
        default:
            current = std::make_shared<Paused>();
            break;
    }
}

void Machine::set_current(std::shared_ptr<State> s)
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
}

// Default response, only classes that change the state will overwrite
void State::pause(Machine *m)
{
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
    // Don't start playing if it wasn't paused
    if (m->m_triggered) {
        m->set_current(std::make_shared<Playing>());
        // delete this handled by shared pointer
        m->m_player->switch_state("Play");

        m->m_triggered = false;
    }
}

//Change state from playing to paused
void Playing::pause(Machine *m)
{
    m->set_current(std::make_shared<Paused>());
    // delete this handled by shared pointer
    m->m_player->switch_state("Pause");

    m->m_triggered = true;
}