#include "state.h"

#include <chrono>

Machine::Machine(std::shared_ptr<Player> player)
    : m_player{player}
{
    fetch();
}

void Machine::fetch()
{
    auto state = m_player->fetch_status();

    if (!current or current->m_state != state)
    {
        switch(state)
        {
            case Player::State::PLAY:
                current = std::make_shared<Playing>();
                break;
            case Player::State::PAUSE:
                current = std::make_shared<Paused>(false);
                break;
            default:
                current = std::make_shared<Paused>(false);
                break;
        }
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
    m_state = Player::State::PLAY;
}

Playing::~Playing()
{
}

Paused::Paused(bool triggered)
    : m_triggered{triggered}
{
    m_state = Player::State::PAUSE;
}

Paused::~Paused()
{
}

//Change state from paused to playing
void Paused::play(Machine *m)
{
    // Don't start playing if it wasn't paused by nancho
    if (m_triggered)
    {
        m->set_current(std::make_shared<Playing>());
        // delete this handled by shared pointer
        m->m_player->switch_state("Play");
    }
}

//Change state from playing to paused
void Playing::pause(Machine *m)
{
    m->set_current(std::make_shared<Paused>(true));
    // delete this handled by shared pointer
    m->m_player->switch_state("Pause");
}