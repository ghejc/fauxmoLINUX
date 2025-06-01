#include <map>
#include <vector>
#include <string>
#include <boost/asio.hpp>
#include <functional>

#pragma once

#define DEBUG_FAUXMO true

#ifdef DEBUG_FAUXMO
#define DEBUG_MSG_FAUXMO(fmt, ...)               \
    {                                            \
        printf(fmt, ##__VA_ARGS__); \
    }
#else
#define DEBUG_MSG_FAUXMO(...)
#endif


class RemoteControl {
public:
    void actionPerformed(unsigned char id, const char *name, bool state, unsigned char value);
    RemoteControl(const std::string &port, unsigned int baud_rate);
    ~RemoteControl();

private:
    void sendBytes(const std::vector<uint8_t> &bytes);

    boost::asio::io_service _io_service;
    boost::asio::serial_port _serial_port;
    std::map<std::string, std::vector<uint8_t>> _remote_control;
    bool _last_state;
    unsigned char _last_value;
};
