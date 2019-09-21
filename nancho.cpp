#include <dbus/dbus.h>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "nancho.h"
#include "player.h"
#include "mpris.h"
#include "state.h"
#include "pulse.h"

#include <memory>
#include <iostream>
#include <cstdlib>
#include <chrono>

using namespace boost;
using namespace boost::program_options;

int main(int argc, char *argv[])
{
    using String_list = std::vector<std::string>;

    options_description desc(
    "Automatically pause music player when a trigger starts playing sound."
    "\n\nAllowed arguments");

    desc.add_options()
        ("help,h",
        "Produce this help message.")

        ("cooldown,c",
        value<std::size_t>()->default_value(0),
        "Time in minutes after which the music will no longer resume. 0 disables cooldown.")
        
        //Boost program_options doesn't play nice with sets
        ("triggers,t",
        value<std::vector<std::string>>()->multitoken(),
        "Names of the binaries that trigger a switch.")

        ("player,p",
        value<std::string>()->default_value("spotify"),
        "Name of the binary that is controlled by the switch.");

    variables_map vm;
    parsed_options parsed_options =command_line_parser(argc, argv).options(desc).run();

    try
    {
        store(parsed_options, vm);
        notify(vm);
    }
    catch (std::exception &e)
    {
        std::cout << std::endl << e.what() << std::endl;
        std::cout << desc << std::endl;

        exit(0);
    }

    if (vm.count("help"))
    {
        std::cout << std::endl;
        std::cout << desc << std::endl;

        exit(0);
    }

    Trigger::Trigger_set binaries_that_trigger_switch;

    if (vm.count("triggers"))
    {
        auto triggers = vm["triggers"].as<String_list>();
        binaries_that_trigger_switch.insert(triggers.begin(), triggers.end());
    }
    else
    {
        binaries_that_trigger_switch.emplace("firefox");
    }

    size_t cooldown = vm["cooldown"].as<size_t>();
    
    std::string player_binary = vm["player"].as<std::string>();

    std::shared_ptr<Player> player = std::make_shared<Mpris>(player_binary);
    std::shared_ptr<Machine> finite_state_machine = std::make_shared<Machine>(player, std::chrono::minutes(cooldown));
    std::shared_ptr<Trigger> trigger = std::make_shared<PulseAudio>(finite_state_machine, binaries_that_trigger_switch);

    int ret = trigger->run();

    return ret;
}
