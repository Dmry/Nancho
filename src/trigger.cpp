#include "trigger.h"

/*
    Seems redundant, but standardizes the way triggers are handled across
    libraries that trigger events. Ensures all those have a common interface
    to trigger the state machine.
*/

std::shared_ptr<Machine> Trigger::m_machine{nullptr};
Trigger::Trigger_set Trigger::m_triggers;

Trigger::Trigger(std::shared_ptr<Machine> fsm, const Trigger_set& triggers)
{
    Trigger::m_triggers = triggers;
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
        case Player::State::UNKNOWN:
            m_machine->fetch();
            break;
        default:
            // do nothing
            break;
    }
}