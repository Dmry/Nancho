#ifndef NANCHO_M_H  
#define NANCHO_M_H

#include <dbus/dbus.h>
#include <pulse/pulseaudio.h>

int pause_spotify(DBusConnection * dbus_conn_, DBusError& dbus_error_);
void worker_thread(DBusConnection * dbus_conn_, DBusError& dbus_error_);
void pulse_callback(pa_context* context, pa_subscription_event_type type, unsigned int, void* user_data);

#endif