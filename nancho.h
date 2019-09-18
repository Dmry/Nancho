#ifndef NANCHO_M_H  
#define NANCHO_M_H

#include <dbus/dbus.h>

int pause_spotify(DBusConnection * dbus_conn_, DBusError& dbus_error_);
void worker_thread(DBusConnection * dbus_conn_, DBusError& dbus_error_);

#endif