#include <dbus/dbus.h>

#include "nancho.h"
#include "player.h"
#include "mpris.h"
#include "state.h"
#include "pulse.h"

#include <memory>

int main(int argc, char *argv[])
{
    Trigger::Trigger_set binaries_that_trigger_switch{"firefox"};

    std::shared_ptr<Player> player = std::make_shared<Mpris>("spotify");
    std::shared_ptr<Machine> finite_state_machine = std::make_shared<Machine>(player);
    std::shared_ptr<Trigger> trigger = std::make_shared<PulseAudio>(finite_state_machine, binaries_that_trigger_switch);

    int ret = trigger->run();

    return ret;
}
