#ifndef TRIGGER_H
#define TRIGGER_H

#include "state.h"
#include "player.h"

#include <memory>
#include <vector>

class Trigger
{
    public:
        using Trigger_set = std::vector<std::string>;
        
        Trigger(std::shared_ptr<Machine> fsm, const Trigger_set& triggers);
        virtual ~Trigger();

        static void trigger(Player::State to_state);
        virtual int run() = 0;

        static std::shared_ptr<Machine> m_machine;
        static Trigger_set m_triggers;
};

#endif