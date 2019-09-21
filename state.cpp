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

    if (!current or current->m_current_state != state)
    {
        switch(state)
        {
            case Player::State::PLAY:
                current = std::make_shared<Playing>(Player::State::UNKNOWN);
                break;
            case Player::State::PAUSE:
                current = std::make_shared<Paused>(Player::State::UNKNOWN);
                break;
            default:
                current = std::make_shared<Paused>(Player::State::UNKNOWN);
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

Playing::Playing(Player::State previous_state)
{
    m_current_state = Player::State::PLAY;
    m_previous_state = previous_state;
}

Playing::~Playing()
{
}

Paused::Paused(Player::State previous_state)
{
    m_current_state = Player::State::PAUSE;
    m_previous_state = previous_state;
}

Paused::~Paused()
{
}

//Change state from paused to playing
void Paused::play(Machine *m)
{
    // Don't start playing if it wasn't paused by nancho
    if (Player::State::PLAY == m_previous_state)
    {
        m->set_current(std::make_shared<Playing>(m_current_state));
        // delete this handled by shared pointer
        m->m_player->switch_state("Play");
    }
}

//Change state from playing to paused
void Playing::pause(Machine *m)
{
    m->set_current(std::make_shared<Paused>(m_current_state));
    // delete this handled by shared pointer
    m->m_player->switch_state("Pause");
}