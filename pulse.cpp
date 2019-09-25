#include "pulse.h"

#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

bool PulseAudio::_switch_guard = false;
std::set<int> PulseAudio::_playing;

PulseAudio::PulseAudio(std::shared_ptr<Machine> fsm, const Trigger_set& triggers)
    : Trigger(fsm, triggers), _mainloop{nullptr}, _mainloop_api{nullptr}, _context{nullptr}, _signal{nullptr}
{
    initialize();
}

bool PulseAudio::initialize()
{
    _mainloop = pa_mainloop_new();
    if (!_mainloop)
    {
        std::cerr << "pa_mainloop_new() failed." << std::endl;
        return false;
    }

    _mainloop_api = pa_mainloop_get_api(_mainloop);

    if (pa_signal_init(_mainloop_api) != 0)
    {
        std::cerr << "pa_signal_init() failed" << std::endl;
        return false;
    }

    _signal = pa_signal_new(SIGINT, exit_signal_callback, this);
    if (!_signal)
    {
        std::cerr << "pa_signal_new() failed" << std::endl;
        return false;
    }
    signal(SIGPIPE, SIG_IGN);

    _context = pa_context_new(_mainloop_api, "Nancho");
    if (!_context)
    {
        std::cerr << "pa_context_new() failed" << std::endl;
        return false;
    }

    if (pa_context_connect(_context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) < 0)
    {
        std::cerr << "pa_context_connect() failed: " << pa_strerror(pa_context_errno(_context)) << std::endl;
        return false;
    }

    pa_context_set_state_callback(_context, context_state_callback, _mainloop_api);

    return true;
}

int PulseAudio::run()
{
    int ret = 1;
    if (pa_mainloop_run(_mainloop, &ret) < 0)
    {
        std::cerr << "pa_mainloop_run() failed." << std::endl;
        return ret;
    }

    return ret;
}

void PulseAudio::quit(int ret)
{
    _mainloop_api->quit(_mainloop_api, ret);
}

void PulseAudio::destroy()
{
    if (_context)
    {
        pa_context_unref(_context);
        _context = NULL;
    }

    if (_signal)
    {
        pa_signal_free(_signal);
        pa_signal_done();
        _signal = NULL;
    }

    if (_mainloop)
    {
        pa_mainloop_free(_mainloop);
        _mainloop = NULL;
        _mainloop_api = NULL;
    }
}

PulseAudio::~PulseAudio()
{
    destroy();
}

void PulseAudio::exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata)
{
    PulseAudio* pa = (PulseAudio*)userdata;
    if (pa) pa->quit();
}

void PulseAudio::server_info_callback(pa_context *c, const pa_server_info *i, void *userdata)
{
    std::cout << "Nancho working on sink : " << i->default_sink_name << std::endl;
}

void PulseAudio::context_state_callback(pa_context *c, void *userdata)
{
    assert(c && userdata);

    PulseAudio* pa = (PulseAudio*)userdata;

    switch (pa_context_get_state(c))
    {
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY:
            std::cout << "Connected Nancho to PulseAudio." << std::endl;
            pa_context_get_server_info(c, server_info_callback, userdata);

            // Subscribe to sink events from the server. This is how we get
            // volume change notifications from the server.
            pa_context_set_subscribe_callback(c, subscribe_callback, userdata);
            pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK_INPUT, NULL, NULL);
            break;

        case PA_CONTEXT_TERMINATED:
            pa->quit(0);
            std::cout << "PulseAudio connection terminated." << std::endl;
            break;

        case PA_CONTEXT_FAILED:

        default:
            std::cerr << "Connection failure: " << pa_strerror(pa_context_errno(c)) << std::endl;
            pa->quit(1);
            break;
    }
}

void PulseAudio::subscribe_callback(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *userdata)
{
    unsigned facility = type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;

    switch (facility)
    {
        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
        {
            pa_context_get_sink_input_info(c, idx, callback, nullptr);          
            break;
        }
        default:
        {
            std::cerr << "WARNING: Received unexpected event" << std::endl; // Got event we aren't expecting.
            break;
        }
    }
}

void PulseAudio::callback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata)
{
    // Apparently PulseAudio triggers each event twice..
    if (!_switch_guard)
    {
        if (!eol)
        {
            std::string app(pa_proplist_gets (i->proplist, "application.process.binary"));

            auto it = m_triggers.find(app);

            if (it != m_triggers.end() and i->volume.values[0] != 0)
            {
                trigger(Player::State::PAUSE);
                _playing.emplace(i->index);
            }

            else if (it != m_triggers.end() and i->volume.values[0] == 0)
            {
                _playing.erase(i->index);
                if (_playing.empty())
                {
                    trigger(Player::State::PLAY);
                }
            }

            // I __know__ this is highly undesirable coupling, but spotify's dbus really leaves me no other option
            else
            {
                // Mpris dbus switches in the order of human time..
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(0.1s);

                trigger(Player::State::UNKNOWN);
            }
        }
        _switch_guard = true;
    }
    else
    {
        _switch_guard = false;
    }
}