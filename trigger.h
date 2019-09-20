#ifndef TRIGGER_H
#define TRIGGER_H

#include "state.h"
#include "player.h"

#include <memory>

class Trigger
{
    public:
        Trigger(std::shared_ptr<Machine> fsm);
        ~Trigger();

        static void trigger(Player::State to_state);
        virtual int run() = 0;

        static std::shared_ptr<Machine> m_machine;
};

#endif