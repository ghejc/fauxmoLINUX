#pragma once

#define FAUXMO_UDP_MULTICAST_IP IPAddress(239, 255, 255, 250)
#define FAUXMO_UDP_MULTICAST_PORT 1900
#define FAUXMO_TCP_PORT 1901
#define FAUXMO_RX_TIMEOUT 3
#define FAUXMO_DEVICE_UNIQUE_ID_LENGTH 27
#define IFACE "wlan1"
#define DEBUG_FAUXMO true
#define DEBUG_FAUXMO_VERBOSE_UDP true
#define DEBUG_FAUXMO_VERBOSE_TCP true

#ifdef DEBUG_FAUXMO
#define DEBUG_MSG_FAUXMO(fmt, ...)               \
    {                                            \
        printf(fmt, ##__VA_ARGS__); \
    }
#else
#define DEBUG_MSG_FAUXMO(...)
#endif

#ifndef DEBUG_FAUXMO_VERBOSE_TCP
#define DEBUG_FAUXMO_VERBOSE_TCP false
#endif

#ifndef DEBUG_FAUXMO_VERBOSE_UDP
#define DEBUG_FAUXMO_VERBOSE_UDP false
#endif

#include <udpserver.hpp>
#include <tcpserver.hpp>
#include <tcpsocket.hpp>
#include <functional>
#include <vector>
#include <cmath>
#include <chrono>
#include <regex>
#include <signal.h>
#include <string>
#include <sstream>
#include <openssl/md5.h>
#include "templates.h"
#include "utils.h"

typedef uint8_t byte;
typedef std::string String;
typedef TCPSocket<> TCPClient;

template <typename T>
String to_string(T x)
{
    std::stringstream ss;
    ss << x;
    return ss.str();
}

template <typename T>
String to_hex(T x)
{
    std::stringstream ss;
    ss << std::hex << x;
    return ss.str();
}

inline double clamp(double x, double lo, double hi)
{
    x = x > lo ? x : lo;
    return x < hi ? x : hi;
};

#define constrain(x, a, b) clamp(x, a, b)

class IPAddress
{
private:
    union
    {
        byte bytes[4]; // IPv4 address
        uint32_t dword;
    } _address;

public:
    // Constructors
    IPAddress()
    {
        _address.bytes[0] = 0;
        _address.bytes[1] = 0;
        _address.bytes[2] = 0;
        _address.bytes[3] = 0;
    };
    IPAddress(byte first_octet, byte second_octet, byte third_octet, byte fourth_octet)
    {
        _address.bytes[0] = first_octet;
        _address.bytes[1] = second_octet;
        _address.bytes[2] = third_octet;
        _address.bytes[3] = fourth_octet;
    };

    String toString() {
        std::stringstream ss;
        ss << uint32_t(_address.bytes[0]) << "." << uint32_t(_address.bytes[1]) << "." << uint32_t(_address.bytes[2]) << "." << uint32_t(_address.bytes[3]);
        return ss.str();  
    }

    bool fromString(const char *address)
    {
        uint16_t acc = 0; // Accumulator
        uint8_t dots = 0;

        while (*address)
        {
            char c = *address++;
            if (c >= '0' && c <= '9')
            {
                acc = acc * 10 + (c - '0');
                if (acc > 255) {
                    // Value out of [0..255] range
                    return false;
                }
            }
            else if (c == '.')
            {
                if (dots == 3) {
                    // Too much dots (there must be 3 dots)
                    return false;
                }
                _address.bytes[dots++] = acc;
                acc = 0;
            }
            else
            {
                // Invalid char
                return false;
            }
        }

        if (dots != 3) {
            // Too few dots (there must be 3 dots)
            return false;
        }
        _address.bytes[3] = acc;
        return true;
    };

    bool operator==(const IPAddress &addr) const { return _address.dword == addr._address.dword; };

    // Overloaded index operator to allow getting and setting individual octets of the address
    byte operator[](int index) const { return _address.bytes[index]; };
    byte &operator[](int index) { return _address.bytes[index]; };
};

typedef std::function<void(unsigned char, const char *, bool, unsigned char)> TSetStateCallback;

typedef struct
{
    char *name;
    bool state;
    unsigned char value;
    byte rgb[3] = {255, 255, 255};
    char uniqueid[FAUXMO_DEVICE_UNIQUE_ID_LENGTH];
} fauxmolinux_device_t;

class fauxmoLINUX
{

public:
    fauxmoLINUX(const String &network_interface);
    ~fauxmoLINUX();

    unsigned char addDevice(const char *device_name);
    bool renameDevice(unsigned char id, const char *device_name);
    bool renameDevice(const char *old_device_name, const char *new_device_name);
    bool removeDevice(unsigned char id);
    bool removeDevice(const char *device_name);
    char *getDeviceName(unsigned char id, char *buffer, size_t len);
    int getDeviceId(const char *device_name);
    void setDeviceUniqueId(unsigned char id, const char *uniqueid);
    void onSetState(TSetStateCallback fn) { _setStateCallback = fn; }
    bool setState(unsigned char id, bool state, unsigned char value);
    bool setState(const char *device_name, bool state, unsigned char value);
    bool setState(unsigned char id, bool state, unsigned char value, byte *rgb);
    bool setState(const char *device_name, bool state, unsigned char value, byte *rgb);
    void enable(bool enable);
    void createServer(bool internal) { _internal = internal; }
    void setPort(unsigned long tcp_port) { _tcp_port = tcp_port; }


private:
    MD5_CTX _context;
    TCPServer<> *_server = NULL;
    bool _enabled = false;
    bool _internal = true;
    unsigned int _tcp_port = FAUXMO_TCP_PORT;
    std::vector<fauxmolinux_device_t> _devices;
    UDPServer<> *_udp = NULL;
    TSetStateCallback _setStateCallback = NULL;
    String _network_interface;

    String _deviceJson(unsigned char id, bool all); // all = true means we are listing all devices so use full description template

    void _onUDPData(const char* message, int length, String ipv4, uint16_t port);
    void _sendUDPResponse(String ipv4, uint16_t port);

    bool _onTCPData(TCPClient *client, void *data, size_t len);
    bool _onTCPRequest(TCPClient *client, bool isGet, String url, String body);
    bool _onTCPDescription(TCPClient *client, String url, String body);
    bool _onTCPList(TCPClient *client, String url, String body);
    bool _onTCPControl(TCPClient *client, String url, String body);
    void _sendTCPResponse(TCPClient *client, const char *code, char *body, const char *mime);

    String _byte2hex(uint8_t zahl);
    String _makeMD5(String text);
    byte *_hs2rgb(uint16_t hue, uint8_t sat);
    byte *_ct2rgb(uint16_t ct);
};
