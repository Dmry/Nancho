#ifndef DBUS_INTERFACE_H
#define DBUS_INTERFACE_H

#include <string>
#include <dbus/dbus.h>

class DBus
{
    public:
        DBus();

        void send(DBusMessage * dbus_msg, DBusMessage ** dbus_reply);
        std::string get_string_reply(DBusMessage * dbus_reply);

    private:
        void connect();
        void recurse(DBusMessageIter *iter);

        DBusError _dbus_error;
        DBusConnection * _dbus_conn;

        std::string m_string_reply;

        enum class DBUS_ERROR {
            ERROR_INIT,
        };
};

#endif