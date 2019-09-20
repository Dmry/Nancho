#include <dbus/dbus.h>

#include "nancho.h"
#include "player.h"
#include "spotify.h"
#include "state.h"
#include "pulse.h"

#include <memory>

/* #include <unistd.h> */

int main(int argc, char *argv[])
{
    std::shared_ptr<Player> player = std::make_shared<Spotify>();
    std::shared_ptr<Machine> finite_state_machine = std::make_shared<Machine>(player);
    std::shared_ptr<Trigger> trigger = std::make_shared<PulseAudio>(finite_state_machine);

    int ret = trigger->run();

    return ret;
}
