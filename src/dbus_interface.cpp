#include "dbus_interface.h"
#include "spdlog/spdlog.h"

#include <iostream>

DBus::DBus()
    : _dbus_error{0}, _dbus_conn{nullptr}, m_string_reply{""}
{
    // Initialize D-Bus error
    dbus_error_init(&_dbus_error);

    // Connect to D-Bus
    connect();
}

void DBus::connect()
{
    if ( nullptr == (_dbus_conn = dbus_bus_get(DBUS_BUS_SESSION, &_dbus_error)) )
    {
        spdlog::error(_dbus_error.name);
        spdlog::error(_dbus_error.message);
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

                m_string_reply = val;

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

std::string DBus::get_string_reply(DBusMessage *dbus_reply)
{
    DBusMessageIter in;
    dbus_message_iter_init(dbus_reply, &in);

    recurse(&in);
    
    return m_string_reply;
}

void DBus::send(DBusMessage * dbus_msg, DBusMessage ** dbus_reply)
{
    dbus_error_free(&_dbus_error);

    // Compose remote procedure call
    if ( nullptr == dbus_msg )
    {
        dbus_connection_unref(_dbus_conn);
        spdlog::error("ERROR: dbus_message_new_method_call - Unable to allocate memory for the message!");
    // Invoke remote procedure call, block for response
    } else if ( nullptr == (*dbus_reply = dbus_connection_send_with_reply_and_block(_dbus_conn, dbus_msg, DBUS_TIMEOUT_USE_DEFAULT, &_dbus_error)) )
    {
        if (!true /*verbose*/)
        {
            dbus_message_unref(dbus_msg);
            spdlog::error(_dbus_error.name);
            spdlog::error(_dbus_error.message);
            std::cerr << "The above means that your target is probably not running" << std::endl;
        }
    }
}