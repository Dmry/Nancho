#include <cxxopts.hpp>
#include <dbus/dbus.h>
#include <spdlog/spdlog.h>

#include "mpris.h"
#include "nancho.h"
#include "player.h"
#include "pulse.h"
#include "state.h"
#include "trigger.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>

int main(int argc, char *argv[])
{
    cxxopts::Options options("Nancho", "Automatically pause music player when a trigger starts playing sound");

    options.add_options()
        ("c,cooldown", "Time in minutes after which the music will no longer resume. 0 disables cooldown", cxxopts::value<size_t>()->default_value("0"))
        ("d,delay", "Time in seconds to wait before pausing music", cxxopts::value<size_t>()->default_value("0"))
        ("trigger", "Names of the binaries that trigger a switch", cxxopts::value<Trigger::Trigger_set>()->default_value("firefox"))
        ("p,player", "Name of the binary that is controlled by the switch", cxxopts::value<std::string>()->default_value("spotify"))
		("debug", "Enable debugging")
        ("h,help", "Print this help message")
    ;

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
      std::cout << options.help() << std::endl;
      exit(0);
    }

	if (result.count("debug"))
	{
		spdlog::set_level(spdlog::level::debug);  
		spdlog::debug("Debug messages enabled");
	}

    auto delay = result["delay"].as<size_t>();
	auto cooldown = result["cooldown"].as<size_t>();
	auto triggers = result["trigger"].as<Trigger::Trigger_set>();
	auto player_str = result["player"].as<std::string>();

	PulseAudio::_delay = std::chrono::seconds(delay);
    std::shared_ptr<Player> player = std::make_shared<Mpris>(player_str);
    std::shared_ptr<Machine> finite_state_machine = std::make_shared<Machine>(player, std::chrono::minutes(cooldown));
    std::shared_ptr<Trigger> trigger = std::make_shared<PulseAudio>(finite_state_machine, triggers);

	spdlog::set_level(spdlog::level::debug);

	spdlog::info("Controlling {}", player_str);
	spdlog::info("Delay is {} seconds, cooldown is {} minutes", delay, cooldown);
	spdlog::info("Listening for triggers:");
	
	for (auto& trigger: triggers)
	{
		spdlog::info("{}", trigger);
	}

    trigger->run();

    return 0;
}