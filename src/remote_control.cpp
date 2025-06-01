#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <cmath>
#include "remote_control.h"

static std::map<std::string, std::string> remote_control_LG = {
    {"on/off", "0x04,0xfb,0x08"},
    {"ch0", "0x04,0xfb,0x10"},
    {"ch1", "0x04,0xfb,0x11"},
    {"ch2", "0x04,0xfb,0x12"},
    {"ch3", "0x04,0xfb,0x13"},
    {"ch4", "0x04,0xfb,0x14"},
    {"ch5", "0x04,0xfb,0x15"},
    {"ch6", "0x04,0xfb,0x16"},
    {"ch7", "0x04,0xfb,0x17"},
    {"ch8", "0x04,0xfb,0x18"},
    {"ch9", "0x04,0xfb,0x19"},
    {"vol+", "0x04,0xfb,0x02"},
    {"vol-", "0x04,0xfb,0x03"},
    {"ch+", "0x04,0xfb,0x00"},
    {"ch-", "0x04,0xfb,0x01"},
    {"mute", "0x04,0xfb,0x09"}
};

static std::map<std::string, std::string> remote_control_DYON = {
    {"on/off", "0x02,0xdf,0x52"},
    {"ch0", "0x02,0xdf,0x1b"},
    {"ch1", "0x02,0xdf,0x00"},
    {"ch2", "0x02,0xdf,0x10"},
    {"ch3", "0x02,0xdf,0x11"},
    {"ch4", "0x02,0xdf,0x13"},
    {"ch5", "0x02,0xdf,0x14"},
    {"ch6", "0x02,0xdf,0x15"},
    {"ch7", "0x02,0xdf,0x17"},
    {"ch8", "0x02,0xdf,0x18"},
    {"ch9", "0x02,0xdf,0x19"},
    {"vol+", "0x02,0xdf,0x03"},
    {"vol-", "0x02,0xdf,0x41"},
    {"ch+", "0x02,0xdf,0x02"},
    {"ch-", "0x02,0xdf,0x09"},
    {"mute", "0x02,0xdf,0x53"}
};

// converts the brightness information received by Alexa to a channel
// 
static std::map<uint8_t, uint8_t> brightness_to_channel_mapping = {
    { 0 , 0 },
    { 1 , 0 },
    { 2 , 0 },
    { 3 , 1 },
    { 4 , 1 },
    { 5 , 1 },
    { 6 , 2 },
    { 7 , 2 },
    { 8 , 2 },
    { 9 , 3 },
    { 10 , 3 },
    { 11 , 4 },
    { 12 , 4 },
    { 13 , 4 },
    { 14 , 5 },
    { 15 , 5 },
    { 16 , 6 },
    { 17 , 6 },
    { 18 , 6 },
    { 19 , 7 },
    { 20 , 7 },
    { 21 , 8 },
    { 22 , 8 },
    { 23 , 8 },
    { 24 , 9 },
    { 25 , 9 },
    { 26 , 10 },
    { 27 , 10 },
    { 28 , 10 },
    { 29 , 11 },
    { 30 , 11 },
    { 31 , 12 },
    { 32 , 12 },
    { 33 , 12 },
    { 34 , 13 },
    { 35 , 13 },
    { 36 , 14 },
    { 37 , 14 },
    { 38 , 14 },
    { 39 , 15 },
    { 40 , 15 },
    { 41 , 16 },
    { 42 , 16 },
    { 43 , 16 },
    { 44 , 17 },
    { 45 , 17 },
    { 46 , 17 },
    { 47 , 18 },
    { 48 , 18 },
    { 49 , 19 },
    { 50 , 19 },
    { 51 , 19 },
    { 52 , 20 },
    { 53 , 20 },
    { 54 , 21 },
    { 55 , 21 },
    { 56 , 21 },
    { 57 , 22 },
    { 58 , 22 },
    { 59 , 23 },
    { 60 , 23 },
    { 61 , 23 },
    { 62 , 24 },
    { 63 , 24 },
    { 64 , 25 },
    { 65 , 25 },
    { 66 , 25 },
    { 67 , 26 },
    { 68 , 26 },
    { 69 , 27 },
    { 70 , 27 },
    { 71 , 27 },
    { 72 , 28 },
    { 73 , 28 },
    { 74 , 29 },
    { 75 , 29 },
    { 76 , 29 },
    { 77 , 30 },
    { 78 , 30 },
    { 79 , 31 },
    { 80 , 31 },
    { 81 , 31 },
    { 82 , 32 },
    { 83 , 32 },
    { 84 , 33 },
    { 85 , 33 },
    { 86 , 33 },
    { 87 , 34 },
    { 88 , 34 },
    { 89 , 34 },
    { 90 , 35 },
    { 91 , 35 },
    { 92 , 36 },
    { 93 , 36 },
    { 94 , 36 },
    { 95 , 37 },
    { 96 , 37 },
    { 97 , 38 },
    { 98 , 38 },
    { 99 , 38 },
    { 100 , 39 },
    { 101 , 39 },
    { 102 , 40 },
    { 103 , 40 },
    { 104 , 40 },
    { 105 , 41 },
    { 106 , 41 },
    { 107 , 42 },
    { 108 , 42 },
    { 109 , 42 },
    { 110 , 43 },
    { 111 , 43 },
    { 112 , 44 },
    { 113 , 44 },
    { 114 , 44 },
    { 115 , 45 },
    { 116 , 45 },
    { 117 , 46 },
    { 118 , 46 },
    { 119 , 46 },
    { 120 , 47 },
    { 121 , 47 },
    { 122 , 48 },
    { 123 , 48 },
    { 124 , 48 },
    { 125 , 49 },
    { 126 , 49 },
    { 127 , 50 },
    { 128 , 50 },
    { 129 , 50 },
    { 130 , 51 },
    { 131 , 51 },
    { 132 , 51 },
    { 133 , 52 },
    { 134 , 52 },
    { 135 , 53 },
    { 136 , 53 },
    { 137 , 53 },
    { 138 , 54 },
    { 139 , 54 },
    { 140 , 55 },
    { 141 , 55 },
    { 142 , 55 },
    { 143 , 56 },
    { 144 , 56 },
    { 145 , 57 },
    { 146 , 57 },
    { 147 , 57 },
    { 148 , 58 },
    { 149 , 58 },
    { 150 , 59 },
    { 151 , 59 },
    { 152 , 59 },
    { 153 , 60 },
    { 154 , 60 },
    { 155 , 61 },
    { 156 , 61 },
    { 157 , 61 },
    { 158 , 62 },
    { 159 , 62 },
    { 160 , 63 },
    { 161 , 63 },
    { 162 , 63 },
    { 163 , 64 },
    { 164 , 64 },
    { 165 , 65 },
    { 166 , 65 },
    { 167 , 65 },
    { 168 , 66 },
    { 169 , 66 },
    { 170 , 66 },
    { 171 , 67 },
    { 172 , 67 },
    { 173 , 68 },
    { 174 , 68 },
    { 175 , 68 },
    { 176 , 69 },
    { 177 , 69 },
    { 178 , 70 },
    { 179 , 70 },
    { 180 , 70 },
    { 181 , 71 },
    { 182 , 71 },
    { 183 , 72 },
    { 184 , 72 },
    { 185 , 72 },
    { 186 , 73 },
    { 187 , 73 },
    { 188 , 74 },
    { 189 , 74 },
    { 190 , 74 },
    { 191 , 75 },
    { 192 , 75 },
    { 193 , 76 },
    { 194 , 76 },
    { 195 , 76 },
    { 196 , 77 },
    { 197 , 77 },
    { 198 , 78 },
    { 199 , 78 },
    { 200 , 78 },
    { 201 , 79 },
    { 202 , 79 },
    { 203 , 80 },
    { 204 , 80 },
    { 205 , 80 },
    { 206 , 81 },
    { 207 , 81 },
    { 208 , 82 },
    { 209 , 82 },
    { 210 , 82 },
    { 211 , 83 },
    { 212 , 83 },
    { 213 , 83 },
    { 214 , 84 },
    { 215 , 84 },
    { 216 , 85 },
    { 217 , 85 },
    { 218 , 85 },
    { 219 , 86 },
    { 220 , 86 },
    { 221 , 87 },
    { 222 , 87 },
    { 223 , 87 },
    { 224 , 88 },
    { 225 , 88 },
    { 226 , 89 },
    { 227 , 89 },
    { 228 , 89 },
    { 229 , 90 },
    { 230 , 90 },
    { 231 , 91 },
    { 232 , 91 },
    { 233 , 91 },
    { 234 , 92 },
    { 235 , 92 },
    { 236 , 93 },
    { 237 , 93 },
    { 238 , 93 },
    { 239 , 94 },
    { 240 , 94 },
    { 241 , 95 },
    { 242 , 95 },
    { 243 , 95 },
    { 244 , 96 },
    { 245 , 96 },
    { 246 , 97 },
    { 247 , 97 },
    { 248 , 97 },
    { 249 , 98 },
    { 250 , 98 },
    { 251 , 99 },
    { 252 , 99 },
    { 253 , 99 },
    { 254 , 99 },
    { 255 , 99 }
};

static std::map<std::string, std::map<std::string, std::string> *> remote_controls = {
    {"LG", &remote_control_LG},
    {"DYON", &remote_control_DYON}
};

void get_remote_control(
    std::map<std::string, std::vector<uint8_t>> &remote_control,
    const std::string &name) {
    std::map<std::string, std::string> *remote_control_ptr = remote_controls[name];


    for (auto &p : *remote_control_ptr) {
        std::stringstream ss;
        ss << p.second;
        std::vector<uint8_t> hex_codes;
        std::string hex_code;
        // transmission instructions see
        // https://server4.eca.ir/eshop/000/other/NEC%20infrared%20codec%20module%20YS-IRTM.pdf
        // default address of the YS-IRTM module
        hex_codes.push_back(0xa1);
        // set the operating mode of the YS-IRTM module to transmit
        hex_codes.push_back(0xf1);
        // add vendor-specific IR codes        
        while(getline(ss, hex_code, ',')) {
            hex_codes.push_back(std::stol(hex_code.c_str(), 0, 16));
        }
        remote_control.insert(std::make_pair(p.first, hex_codes));
    }
}

void RemoteControl::sendBytes(const std::vector<uint8_t> &bytes) {
	// send byte sequence over the serial port
	// std::vector<uint8_t> data = {0x04, 0xfb, 0x08};
	// sendBytes(data);
    DEBUG_MSG_FAUXMO("Sending %d bytes: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
        bytes.size(), bytes[0], bytes[1], bytes[2], bytes[3], bytes[4])
    _serial_port.write_some(boost::asio::buffer(bytes, bytes.size()));
};

void get_digits(std::vector<uint8_t>& digits, uint8_t num) {
    if (num > 9) {
        get_digits(digits, num / 10);
    }
    digits.push_back(num % 10);
}

RemoteControl::RemoteControl(const std::string &port, unsigned int baud_rate) : _io_service(), _serial_port(_io_service, port), _last_state(false), _last_value(255) {
    _serial_port.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
	_serial_port.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none));
	_serial_port.set_option(boost::asio::serial_port::character_size(boost::asio::serial_port::character_size(8)));
	_serial_port.set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one));
	_serial_port.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none));
    get_remote_control(_remote_control, "LG");
}


RemoteControl::~RemoteControl() {
    _serial_port.close();
}

uint8_t convert_brightness_to_channel(uint8_t value) {
    return brightness_to_channel_mapping.at(value);
}

void RemoteControl::actionPerformed(unsigned char device_id, const char *device_name, bool state, unsigned char value) {

    DEBUG_MSG_FAUXMO("Perform action for device %s[%d] -> state %d, value %d\n", device_name, device_id, state, value)

    if (!strcmp(device_name, "fernseher")) {

        // handling switch on/off
        if (_remote_control.count("on/off")) {
            if (state != _last_state) {
                DEBUG_MSG_FAUXMO("State transition from %d to %d\n", _last_state, state)
                sendBytes(_remote_control.at("on/off"));
                _last_state = state;
            }
        }

        // handling channel selection
        if (value > 0 && (value != _last_value)) {
            _last_value = value;
            std::vector<uint8_t> digits;
            uint8_t channel = convert_brightness_to_channel(value);
            DEBUG_MSG_FAUXMO("Set %s to channel %d\n", device_name, channel)
            get_digits(digits, channel);
            DEBUG_MSG_FAUXMO("Channel %d has %d digits\n", channel, digits.size())
            for (auto digit : digits) {
                std::stringstream ss;
                ss << "ch" << std::dec << int(digit);
                std::string cmd = ss.str();
                DEBUG_MSG_FAUXMO("Set %s to channel %s\n", device_name, cmd.c_str())
                if (_remote_control.count(cmd)) {
                    sendBytes(_remote_control.at(cmd));
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }   
            }
        }
    }        
}
