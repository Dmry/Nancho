#include "nancho.h"

/*
    TODO:
        - State machine to check if spotify was playing when switch is signaled
        - Delay switch for hickups in streams or don't switch at all after like 20 min video?
*/

#include "dbus-print-message.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <sys/wait.h>

DBus* DBus::inst = nullptr;

DBus::DBus()
    : _dbus_error{0}, _dbus_conn{nullptr}, last_status{""}
{
    initialize();
}

DBus *DBus::instance()
{
    if (!inst)
        inst = new DBus();
    return inst;
}

void DBus::initialize()
{
    // Initialize D-Bus error
    dbus_error_init(&_dbus_error);

    // Connect to D-Bus
    if ( nullptr == (_dbus_conn = dbus_bus_get(DBUS_BUS_SESSION, &_dbus_error)) )
    {
        perror(_dbus_error.name);
        perror(_dbus_error.message);
        throw DBUS_ERROR::ERROR_INIT;
    }
}

void DBus::recurse(DBusMessageIter *iter)
{
    do {
        int type = dbus_message_iter_get_arg_type (iter);

        switch(type) {
            case DBUS_TYPE_VARIANT:
            {
	            DBusMessageIter subiter;

	            dbus_message_iter_recurse (iter, &subiter);

	            recurse (&subiter);
	            break;
            }
            case DBUS_TYPE_STRING:
            {
	            char *val;
	            dbus_message_iter_get_basic (iter, &val);

                last_status = val;

                return;

	            break;
            }
            case DBUS_TYPE_INVALID:
            {
	            break;
            }
        }
    } while (dbus_message_iter_next (iter));
}

void DBus::send(DBusMessage * dbus_msg, DBusMessage ** dbus_reply)
{
    dbus_error_free(&_dbus_error);

    // Compose remote procedure call
    if ( nullptr == dbus_msg )
    {
        dbus_connection_unref(_dbus_conn);
        perror("ERROR: dbus_message_new_method_call - Unable to allocate memory for the message!");
    // Invoke remote procedure call, block for response
    } else if ( nullptr == (*dbus_reply = dbus_connection_send_with_reply_and_block(_dbus_conn, dbus_msg, DBUS_TIMEOUT_USE_DEFAULT, &_dbus_error)) )
    {
        if (!true /*verbose*/)
        {
            dbus_message_unref(dbus_msg);
            perror(_dbus_error.name);
            perror(_dbus_error.message);
            std::cerr << "The above means that you're probably not running Spotify" << std::endl;
        }
    }
}

void DBus::switch_state(std::string command)
{
    DBusMessage * dbus_reply = nullptr;
    
    DBusMessage * dbus_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.spotify", "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", command.c_str());

    send(dbus_msg, &dbus_reply);
}

std::string DBus::fetch_status()
{
    DBusMessage * dbus_reply = nullptr;
    DBusMessage * dbus_msg = dbus_message_new_method_call("org.mpris.MediaPlayer2.spotify", "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");

    const char *player = "org.mpris.MediaPlayer2.Player";
    const char *status = "PlaybackStatus";

    dbus_message_append_args (dbus_msg, DBUS_TYPE_STRING, &player, DBUS_TYPE_STRING, &status, DBUS_TYPE_INVALID);

    send(dbus_msg, &dbus_reply);

    if (dbus_reply)
    {
        DBusMessageIter in;
        dbus_message_iter_init(dbus_reply, &in);

        recurse(&in);
    }

    return last_status;
}

PulseAudio::PulseAudio()
    : _mainloop{nullptr}, _mainloop_api{nullptr}, _context{nullptr}, _signal{nullptr}
{
    if (DBus::instance()->fetch_status() == "Playing")
        PulseAudio::spotify_was_playing = true;
}

bool PulseAudio::spotify_was_playing{false};


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

    pa_operation *op = NULL;

    switch (facility)
    {
        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
            pause_play(c); 
            break;
        default:
            std::cout << "this " << std::endl;
            assert(0); // Got event we aren't expecting.
            break;
    }

    if (op)
        pa_operation_unref(op);
}

void PulseAudio::pause_play(pa_context *c)
{
    if (false == spotify_was_playing)
    {
        pa_operation* op = pa_context_get_sink_input_info_list(c, callback_was_not_playing, nullptr);
        if (op)
            pa_operation_unref(op);
    }
    else
    {
        pa_operation* op = pa_context_get_sink_input_info_list(c, callback_was_playing, nullptr);
        if (op)
            pa_operation_unref(op);
    }
}

void PulseAudio::server_info_callback(pa_context *c, const pa_server_info *i, void *userdata)
{
    std::cout << "Nancho working on sink : " << i->default_sink_name << std::endl;
}

void PulseAudio::callback_was_not_playing(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata)
{
    if (!eol)
    {
        std::string app(pa_proplist_gets (i->proplist, "application.process.binary"));

        if (app == "firefox" and i->volume.values[0] != 0)
        {
            if( DBus::instance()->fetch_status() == "Playing" )
            {
                spotify_was_playing = true;
                DBus::instance()->switch_state("Pause");
            }
            if( DBus::instance()->fetch_status() == "Paused" )
            {
                spotify_was_playing = true;
            }
        }
    }
}

void PulseAudio::callback_was_playing(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata)
{
    if (!eol)
    {
        std::string app(pa_proplist_gets (i->proplist, "application.process.binary"));

        if (app == "firefox" and i->volume.values[0] == 0)
        {
            if( DBus::instance()->fetch_status() == "Playing" )
            {
                spotify_was_playing = false;
            }
            if( DBus::instance()->fetch_status() == "Paused" )
            {
                spotify_was_playing = false;
                DBus::instance()->switch_state("Play");
            }
        }
    }
}

int main(int argc, char *argv[])
{
    PulseAudio pa = PulseAudio();
    if (!pa.initialize())
        return 0;

    int ret = pa.run();

    return ret;
}
