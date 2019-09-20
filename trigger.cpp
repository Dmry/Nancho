#include "trigger.h"

std::shared_ptr<Machine> Trigger::m_machine{nullptr};

Trigger::Trigger(std::shared_ptr<Machine> fsm)
{
    Trigger::m_machine = fsm;
}

Trigger::~Trigger()
{
}

void Trigger::trigger(Player::State to_state)
{
    switch (to_state)
    {
        case Player::State::PLAY:
            m_machine->play();
            break;
        case Player::State::PAUSE:
            m_machine->pause();
            break;
        default:
            // do nothing
            break;
    }
}