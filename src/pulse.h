#ifndef PULSE_H
#define PULSE_H

#include <pulse/pulseaudio.h>

#include "trigger.h"

#include <csignal>
#include <set>

class PulseAudio : public Trigger
{
    private:
        pa_mainloop* _mainloop;
        pa_mainloop_api* _mainloop_api;
        pa_context* _context;
        pa_signal_event* _signal;
        static std::set<int> _playing;

    public:
        PulseAudio(std::shared_ptr<Machine> fsm, const Trigger_set& triggers);
        ~PulseAudio();

        bool initialize();
        int run() override;
        void quit(int ret = 0);
        void destroy();
        
        static std::chrono::seconds _delay;

    private:
        static void exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata);
        static void context_state_callback(pa_context *c, void *userdata);
        static void subscribe_callback(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *userdata);
        static void server_info_callback(pa_context *c, const pa_server_info *i, void *userdata);
        static void callback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata);
        static void callback_execute(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata);
};

#endif