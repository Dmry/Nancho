#include <dbus/dbus.h>

#include "nancho.h"
#include "player.h"
#include "mpris.h"
#include "state.h"
#include "pulse.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace boost;
using namespace boost::program_options;

#include <memory>
#include <iostream>

int main(int argc, char *argv[])
{
    using String_list = std::vector<std::string>;

    options_description desc(
    "\nAutomatically pause music player when a trigger starts playing sound."
    "\n\nAllowed arguments");

    desc.add_options()
        ("help,h",
        "Produce this help message.")
        
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
    }

    if (vm.count("help"))
    {
        std::cout << std::endl;
        std::cout << desc << std::endl;
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
    
    std::string player_binary = vm["player"].as<std::string>();

    std::shared_ptr<Player> player = std::make_shared<Mpris>(player_binary);
    std::shared_ptr<Machine> finite_state_machine = std::make_shared<Machine>(player);
    std::shared_ptr<Trigger> trigger = std::make_shared<PulseAudio>(finite_state_machine, binaries_that_trigger_switch);

    int ret = trigger->run();

    return ret;
}
