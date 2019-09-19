#ifndef NANCHO_M_H  
#define NANCHO_M_H

#include <string>
#include <dbus/dbus.h>
#include <csignal>
#include <pulse/pulseaudio.h>

class DBus
{
    public:
        static DBus *instance();
        void switch_state(std::string);
        std::string fetch_status();

    protected:

        DBus();
        void initialize();

    private:
        DBusError _dbus_error;
        DBusConnection * _dbus_conn;
        void recurse(DBusMessageIter *iter);
        void send(DBusMessage * dbus_msg, DBusMessage ** dbus_reply);

        static DBus *inst;
        std::string last_status;

        enum class DBUS_ERROR {
            ERROR_INIT,
        };
};

class PulseAudio
{
    private:
        pa_mainloop* _mainloop;
        pa_mainloop_api* _mainloop_api;
        pa_context* _context;
        pa_signal_event* _signal;
        static bool spotify_was_playing;

    public:
        PulseAudio();

        bool initialize();
        int run();
        void quit(int ret = 0);
        void destroy();
        ~PulseAudio();

    private:

        static void exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata);
        static void context_state_callback(pa_context *c, void *userdata);
        static void subscribe_callback(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *userdata);
        static void pause_play(pa_context *c);
        static void server_info_callback(pa_context *c, const pa_server_info *i, void *userdata);
        static void callback_was_playing(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata);
        static void callback_was_not_playing(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata);
};

#endif