#include <dbus/dbus.h>
#include "args.hpp"

#include "nancho.h"
#include "player.h"
#include "trigger.h"
#include "mpris.h"
#include "state.h"
#include "pulse.h"

#include <memory>
#include <iostream>
#include <cstdlib>
#include <chrono>

struct nancho : args::group<nancho>
{
    static const char* help()
    {
        return "Automatically pause music player when a trigger starts playing sound.";
    }

    std::string player_binary;
    Trigger::Trigger_set binaries_that_trigger_switch;
    size_t cooldown;
    size_t delay;
    
    nancho() :
        player_binary{},
        binaries_that_trigger_switch{},
        cooldown{0},
        delay{0}
    {}

    template<class F>
    void parse(F f)
    {
        f(cooldown, "-c", "--cooldown", args::help("Time in minutes after which the music will no longer resume. 0 disables cooldown."));
        f(delay, "-d", "--delay", args::help("Time in seconds to wait before pausing and resuming music."));
        f(binaries_that_trigger_switch, "-t", "--trigger", args::help("Names of the binaries that trigger a switch."));
        f(player_binary, "-p", "--player", args::help("Name of the binary that is controlled by the switch."));
    }

    void run()
    {
        if (binaries_that_trigger_switch.empty())
        {
            binaries_that_trigger_switch.insert("firefox");
        }

        if (player_binary.empty())
        {
            player_binary = "spotify";
        }

        std::shared_ptr<Player> player = std::make_shared<Mpris>(player_binary);
        std::shared_ptr<Machine> finite_state_machine = std::make_shared<Machine>(player, std::chrono::minutes(cooldown));
        std::shared_ptr<Trigger> trigger = std::make_shared<PulseAudio>(finite_state_machine, binaries_that_trigger_switch, std::chrono::seconds(delay));

        trigger->run();

        return;
    }
};

int main(int argc , const char* argv[])
{
    args::parse<nancho>(argc, argv);

    return 0;
}
