#include "state.h"

Machine::Machine(std::shared_ptr<Player> player, std::chrono::minutes cooldown, std::chrono::seconds delay)
    : m_player{player}, m_cooldown{cooldown}, m_delay{delay}
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
    auto f = std::async(std::launch::async, [this] () {
        std::this_thread::sleep_for(m_delay);
        current->play(this);
    });
}

void Machine::pause()
{
    auto f = std::async(std::launch::async, [this] () {
        std::this_thread::sleep_for(m_delay);
        current->pause(this);
    });
}

// Default response, only classes that change the state will overwrite
void State::play(Machine *m [[maybe_unused]])
{
}

// Default response, only classes that change the state will overwrite
void State::pause(Machine *m [[maybe_unused]])
{
}

Playing::Playing(Player::State previous_state)
{
    m_current_state = Player::State::PLAY;
    m_previous_state = previous_state;
    m_time_switched = std::chrono::steady_clock::now();
}

Playing::~Playing()
{
}

Paused::Paused(Player::State previous_state)
{
    m_current_state = Player::State::PAUSE;
    m_previous_state = previous_state;
    m_time_switched = std::chrono::steady_clock::now();
}

Paused::~Paused()
{
}

//Change state from paused to playing
void Paused::play(Machine *m)
{
    bool cooldown_passed{false};

    if (m->m_cooldown != std::chrono::minutes(0))
    {
        m->m_cooldown > std::chrono::steady_clock::now() - m_time_switched ? cooldown_passed = false : cooldown_passed = true; 
    }

    // Don't start playing if it wasn't paused by nancho
    if (Player::State::PAUSE == m_current_state
        and Player::State::PLAY == m_previous_state
        and not cooldown_passed)
    {
        m->set_current(std::make_shared<Playing>(m_current_state));
        // delete this handled by shared pointer
        m->m_player->switch_state("Play");
    }
}

//Change state from playing to paused
void Playing::pause(Machine *m)
{
    if (Player::State::PLAY == m_current_state
        or Player::State::UNKNOWN == m_current_state)
    {
        m->set_current(std::make_shared<Paused>(m_current_state));
        // delete this handled by shared pointer
        m->m_player->switch_state("Pause");
    }
}