#include "fauxmoLINUX.h"
#include "remote_control.h"

// -----------------------------------------------------------------------------
// UDP
// -----------------------------------------------------------------------------

void fauxmoLINUX::_sendUDPResponse(String ipv4, uint16_t port)
{

	DEBUG_MSG_FAUXMO("[FAUXMO] Responding to M-SEARCH request\n");

	IPAddress ip;
	ip.fromString(get_ip_addr(_network_interface.c_str()));
	String mac = String(get_mac_addr(_network_interface.c_str()));
	std::regex_replace(mac, std::regex(":"), "");
	std::transform(mac.begin(), mac.end(), mac.begin(),
				   [](unsigned char c)
				   { return std::tolower(c); });

	char response[strlen(FAUXMO_UDP_RESPONSE_TEMPLATE) + 128];
	snprintf(
		response, sizeof(response),
		FAUXMO_UDP_RESPONSE_TEMPLATE,
		ip[0], ip[1], ip[2], ip[3],
		_tcp_port,
		mac.c_str(), mac.c_str());

#if DEBUG_FAUXMO_VERBOSE_UDP
	DEBUG_MSG_FAUXMO("[FAUXMO] UDP response sent to %s:%d\n%s", ipv4.c_str(), port, response);
#endif

	_udp->SendTo(response, strlen(response), ipv4, port);
}

void fauxmoLINUX::_onUDPData(const char *message, int length, String ipv4, uint16_t port)
{

	if (length > 0)
	{

#if DEBUG_FAUXMO_VERBOSE_UDP
		DEBUG_MSG_FAUXMO("[FAUXMO] UDP packet received\n%s", message);
#endif

		String request = (const char *)message;
		if (request.find("M-SEARCH") != String::npos)
		{
			if ((request.find("ssdp:discover") != String::npos) || (request.find("upnp:rootdevice") != String::npos) || (request.find("device:basic:1") != String::npos))
			{
				_sendUDPResponse(ipv4, port);
			}
		}
	}
}

// -----------------------------------------------------------------------------
// TCP
// -----------------------------------------------------------------------------

void fauxmoLINUX::_sendTCPResponse(TCPClient *client, const char *code, char *body, const char *mime)
{

	char headers[strlen(FAUXMO_TCP_HEADERS) + 32];
	snprintf(
		headers, sizeof(headers),
		FAUXMO_TCP_HEADERS,
		code, mime, strlen(body));

#if DEBUG_FAUXMO_VERBOSE_TCP
	DEBUG_MSG_FAUXMO("[FAUXMO] Response:\n%s%s\n", headers, body);
#endif

	client->Send(headers);
	client->Send(body);
}

String fauxmoLINUX::_deviceJson(unsigned char id, bool all = true)
{

	if (id >= _devices.size())
		return "{}";

	fauxmolinux_device_t device = _devices[id];

	DEBUG_MSG_FAUXMO("[FAUXMO] Sending device info for \"%s\", uniqueID = \"%s\"\n", device.name, device.uniqueid);
	char buffer[strlen(FAUXMO_DEVICE_JSON_TEMPLATE) + 64];

	if (all)
	{
		snprintf(
			buffer, sizeof(buffer),
			FAUXMO_DEVICE_JSON_TEMPLATE,
			device.name, device.uniqueid,
			device.state ? "true" : "false",
			device.value);
	}
	else
	{
		snprintf(
			buffer, sizeof(buffer),
			FAUXMO_DEVICE_JSON_TEMPLATE_SHORT,
			device.name, device.uniqueid);
	}

	return String(buffer);
}

String fauxmoLINUX::_byte2hex(uint8_t zahl)
{
	String hstring = to_hex<uint32_t>(zahl);
	if (zahl < 16)
	{
		hstring = "0" + hstring;
	}

	return hstring;
}

String fauxmoLINUX::_makeMD5(String text)
{
	unsigned char bbuf[16];
	String hash = "";
	MD5_Init(&_context);
	MD5_Update(&_context, text.c_str(), text.length());
	MD5_Final(bbuf, &_context);

	for (uint8_t i = 0; i < 16; i++)
	{
		hash += _byte2hex(bbuf[i]);
	}

	return hash;
}

bool fauxmoLINUX::_onTCPDescription(TCPClient *client, String url, String body)
{
	DEBUG_MSG_FAUXMO("[FAUXMO] _onTCPDescription(%s,%s,%s)\n",
		client->remoteAddress().c_str(),url.c_str(),body.c_str());

	(void)url;
	(void)body;

	DEBUG_MSG_FAUXMO("[FAUXMO] Handling /description.xml request\n");

	IPAddress ip;
	ip.fromString(get_ip_addr(_network_interface.c_str()));
	String mac = String(get_mac_addr(_network_interface.c_str()));
	std::regex_replace(mac, std::regex(":"), "");
	std::transform(mac.begin(), mac.end(), mac.begin(),
				   [](unsigned char c)
				   { return std::tolower(c); });

	char response[strlen(FAUXMO_DESCRIPTION_TEMPLATE) + 64];
	snprintf(
		response, sizeof(response),
		FAUXMO_DESCRIPTION_TEMPLATE,
		ip[0], ip[1], ip[2], ip[3], _tcp_port,
		ip[0], ip[1], ip[2], ip[3], _tcp_port,
		mac.c_str(), mac.c_str());

	_sendTCPResponse(client, "200 OK", response, "text/xml");

	return true;
}

bool fauxmoLINUX::_onTCPList(TCPClient *client, String url, String body)
{
	DEBUG_MSG_FAUXMO("[FAUXMO] _onTCPList(%s,%s,%s)\n",
		client->remoteAddress().c_str(),url.c_str(),body.c_str());

	DEBUG_MSG_FAUXMO("[FAUXMO] Handling list request\n");

	// Get the index
	int pos = url.find("lights");
	if (String::npos == pos)
		return false;

	// Get the id
	unsigned char id = 0;
	if (pos + 7 < url.length())
	{
		id = atoi(url.substr(pos + 7).c_str());
	}

	// This will hold the response string
	String response;

	if (0 == id)
	{
		// Client is requesting all devices
		response += "{";
		for (unsigned char i = 0; i < _devices.size(); i++)
		{
			if (i > 0)
				response += ",";
			response += "\"" + to_string<uint32_t>(i + 1) + "\":" + _deviceJson(i, false); // send short template
		}
		response += "}";

	}
	else
	{
		// Client is requesting a single device
		response = _deviceJson(id - 1);
	}

	_sendTCPResponse(client, "200 OK", (char *)response.c_str(), "application/json");

	return true;
}

byte *fauxmoLINUX::_hs2rgb(uint16_t hue, uint8_t sat)
{
	byte *rgb = new byte[3]{0, 0, 0};

	float h = ((float)hue) / 65535.0;
	float s = ((float)sat) / 255.0;

	byte i = floor(h * 6);
	float f = h * 6 - i;
	float p = 255 * (1 - s);
	float q = 255 * (1 - f * s);
	float t = 255 * (1 - (1 - f) * s);
	switch (i % 6)
	{
	case 0:
		rgb[0] = 255, rgb[1] = t, rgb[2] = p;
		break;
	case 1:
		rgb[0] = q, rgb[1] = 255, rgb[2] = p;
		break;
	case 2:
		rgb[0] = p, rgb[1] = 255, rgb[2] = t;
		break;
	case 3:
		rgb[0] = p, rgb[1] = q, rgb[2] = 255;
		break;
	case 4:
		rgb[0] = t, rgb[1] = p, rgb[2] = 255;
		break;
	case 5:
		rgb[0] = 255, rgb[1] = p, rgb[2] = q;
	}
	return rgb;
}

byte *fauxmoLINUX::_ct2rgb(uint16_t ct)
{
	byte *rgb = new byte[3]{0, 0, 0};
	float temp = 10000 / ct; // kelvins = 1,000,000/mired (and that /100)
	float r, g, b;

	if (temp <= 66)
	{
		r = 255;
		g = temp;
		g = 99.470802 * log(g) - 161.119568;
		if (temp <= 19)
		{
			b = 0;
		}
		else
		{
			b = temp - 10;
			b = 138.517731 * log(b) - 305.044793;
		}
	}
	else
	{
		r = temp - 60;
		r = 329.698727 * pow(r, -0.13320476);
		g = temp - 60;
		g = 288.12217 * pow(g, -0.07551485);
		b = 255;
	}

	rgb[0] = (byte)constrain(r, 0.1, 255.1);
	rgb[1] = (byte)constrain(g, 0.1, 255.1);
	rgb[2] = (byte)constrain(b, 0.1, 255.1);

	return rgb;
}

bool fauxmoLINUX::_onTCPControl(TCPClient *client, String url, String body)
{
	DEBUG_MSG_FAUXMO("[FAUXMO] _onTCPControl(%s,%s,%s)\n",
		client->remoteAddress().c_str(),url.c_str(),body.c_str());

	// "devicetype" request
	if (body.find("devicetype") != String::npos)
	{
		DEBUG_MSG_FAUXMO("[FAUXMO] Handling devicetype request\n");
		_sendTCPResponse(client, "200 OK", (char *)"[{\"success\":{\"username\": \"2WLEDHardQrI3WHYTHoMcXHgEspsM8ZZRpSKtBQr\"}}]", "application/json");
		return true;
	}

	// "state" request
	if ((url.find("state") != String::npos) && (body.length() > 0))
	{

		// Get the index
		int pos = url.find("lights");
		if (-1 == pos)
			return false;

		DEBUG_MSG_FAUXMO("[FAUXMO] Handling state request\n");

		// Get the index
		unsigned char id = atoi(url.substr(pos + 7).c_str());
		if (id > 0)
		{

			--id;

			// Brightness
			if ((pos = body.find("bri")) != String::npos)
			{
				unsigned char value = atoi(body.substr(pos + 5).c_str());
				_devices[id].value = value;
				_devices[id].state = (value > 0);
			}
			else if ((pos = body.find("hue")) != String::npos)
			{
				_devices[id].state = true;
				unsigned int pos_comma = body.find(",", pos);
				uint16_t hue = atoi(body.substr(pos + 5, pos_comma).c_str());
				pos = body.find("sat", pos_comma);
				uint8_t sat = atoi(body.substr(pos + 5).c_str());
				byte *rgb = _hs2rgb(hue, sat);
				_devices[id].rgb[0] = rgb[0];
				_devices[id].rgb[1] = rgb[1];
				_devices[id].rgb[2] = rgb[2];
			}
			else if ((pos = body.find("ct")) != String::npos)
			{
				_devices[id].state = true;
				uint16_t ct = atoi(body.substr(pos + 4).c_str());
				byte *rgb = _ct2rgb(ct);
				_devices[id].rgb[0] = rgb[0];
				_devices[id].rgb[1] = rgb[1];
				_devices[id].rgb[2] = rgb[2];
			}
			else if (body.find("false") != String::npos)
			{
				_devices[id].state = false;
			}
			else
			{
				_devices[id].state = true;
				if (0 == _devices[id].value)
					_devices[id].value = 255;

			}

			char response[strlen(FAUXMO_TCP_STATE_RESPONSE) + 10];
			snprintf(
				response, sizeof(response),
				FAUXMO_TCP_STATE_RESPONSE,
				id + 1, _devices[id].state ? "true" : "false");
			_sendTCPResponse(client, "200 OK", response, "text/xml");

			if (_setStateCallback)
			{
				_setStateCallback(id, _devices[id].name, _devices[id].state, _devices[id].value);
			}

			return true;
		}
	}

	return false;
}

bool fauxmoLINUX::_onTCPRequest(TCPClient *client, bool isGet, String url, String body)
{
	if (!_enabled)
		return false;

#if DEBUG_FAUXMO_VERBOSE_TCP
	DEBUG_MSG_FAUXMO("[FAUXMO] isGet: %s\n", isGet ? "true" : "false");
	DEBUG_MSG_FAUXMO("[FAUXMO] URL: %s\n", url.c_str());
	if (!isGet)
		DEBUG_MSG_FAUXMO("[FAUXMO] Body:\n%s\n", body.c_str());
#endif

	if (url == "/description.xml")
	{
		return _onTCPDescription(client, url, body);
	}

	if (url.rfind("/api", 0) == 0)
	{
		if (isGet)
		{
			return _onTCPList(client, url, body);
		}
		else
		{
			return _onTCPControl(client, url, body);
		}
	}

	return false;
}

bool fauxmoLINUX::_onTCPData(TCPClient *client, void *data, size_t len)
{

	if (!_enabled)
		return false;

	char *p = (char *)data;
	p[len] = 0;

#if DEBUG_FAUXMO_VERBOSE_TCP
	DEBUG_MSG_FAUXMO("[FAUXMO] TCP request\n%s\n", p);
#endif

	// Method is the first word of the request
	char *method = p;

	while (*p != ' ')
		p++;
	*p = 0;
	p++;

	// Split word and flag start of url
	char *url = p;

	// Find next space
	while (*p != ' ')
		p++;
	*p = 0;
	p++;

	// Find double line feed
	unsigned char c = 0;
	while ((*p != 0) && (c < 2))
	{
		if (*p != '\r')
		{
			c = (*p == '\n') ? c + 1 : 0;
		}
		p++;
	}
	char *body = p;

	bool isGet = (strncmp(method, "GET", 3) == 0);

	return _onTCPRequest(client, isGet, url, body);
}

// -----------------------------------------------------------------------------
// Devices
// -----------------------------------------------------------------------------

fauxmoLINUX::fauxmoLINUX(const String &network_interface):_network_interface(network_interface)
{
}

fauxmoLINUX::~fauxmoLINUX()
{

	// Free the name for each device
	for (auto &device : _devices)
	{
		free(device.name);
	}

	// Delete devices
	_devices.clear();
}

void fauxmoLINUX::setDeviceUniqueId(unsigned char id, const char *uniqueid)
{
	strncpy(_devices[id].uniqueid, uniqueid, FAUXMO_DEVICE_UNIQUE_ID_LENGTH);
}

unsigned char fauxmoLINUX::addDevice(const char *device_name)
{

	fauxmolinux_device_t device;
	unsigned int device_id = _devices.size();

	// init properties
	device.name = strdup(device_name);
	device.state = false;
	device.value = 0;

	// create the uniqueid
	String mac = String(get_mac_addr(_network_interface.c_str()));

	snprintf(device.uniqueid, FAUXMO_DEVICE_UNIQUE_ID_LENGTH, "%02X:%s:%s", device_id, mac.c_str(), "00:00");

	// Attach
	_devices.push_back(device);

	DEBUG_MSG_FAUXMO("[FAUXMO] Device '%s' added as #%d\n", device_name, device_id);

	return device_id;
}

int fauxmoLINUX::getDeviceId(const char *device_name)
{
	for (unsigned int id = 0; id < _devices.size(); id++)
	{
		if (strcmp(_devices[id].name, device_name) == 0)
		{
			return id;
		}
	}
	return -1;
}

bool fauxmoLINUX::renameDevice(unsigned char id, const char *device_name)
{
	if (id < _devices.size())
	{
		free(_devices[id].name);
		_devices[id].name = strdup(device_name);
		DEBUG_MSG_FAUXMO("[FAUXMO] Device #%d renamed to '%s'\n", id, device_name);
		return true;
	}
	return false;
}

bool fauxmoLINUX::renameDevice(const char *old_device_name, const char *new_device_name)
{
	int id = getDeviceId(old_device_name);
	if (id < 0)
		return false;
	return renameDevice(id, new_device_name);
}

bool fauxmoLINUX::removeDevice(unsigned char id)
{
	if (id < _devices.size())
	{
		free(_devices[id].name);
		_devices.erase(_devices.begin() + id);
		DEBUG_MSG_FAUXMO("[FAUXMO] Device #%d removed\n", id);
		return true;
	}
	return false;
}

bool fauxmoLINUX::removeDevice(const char *device_name)
{
	int id = getDeviceId(device_name);
	if (id < 0)
		return false;
	return removeDevice(id);
}

char *fauxmoLINUX::getDeviceName(unsigned char id, char *device_name, size_t len)
{
	if ((id < _devices.size()) && (device_name != NULL))
	{
		strncpy(device_name, _devices[id].name, len);
	}
	return device_name;
}

bool fauxmoLINUX::setState(unsigned char id, bool state, unsigned char value)
{
	if (id < _devices.size())
	{
		_devices[id].state = state;
		_devices[id].value = value;
		return true;
	}
	return false;
}

bool fauxmoLINUX::setState(const char *device_name, bool state, unsigned char value)
{
	return setState(getDeviceId(device_name), state, value);
}

bool fauxmoLINUX::setState(unsigned char id, bool state, unsigned char value, byte *rgb)
{
	if (id >= _devices.size())
		return false;
	bool success = setState(id, state, value);
	if (success)
	{
		_devices[id].rgb[0] = rgb[0];
		_devices[id].rgb[1] = rgb[1];
		_devices[id].rgb[2] = rgb[2];
	}
	return success;
}

bool fauxmoLINUX::setState(const char *device_name, bool state, unsigned char value, byte *rgb)
{
	return setState(getDeviceId(device_name), state, value, rgb);
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

void fauxmoLINUX::enable(bool enable)
{

	if (enable == _enabled)
		return;
	_enabled = enable;
	if (_enabled)
	{
		DEBUG_MSG_FAUXMO("[FAUXMO] Enabled\n");
	}
	else
	{
		DEBUG_MSG_FAUXMO("[FAUXMO] Disabled\n");
	}

	if (_enabled)
	{

		// Start TCP server if internal
		if (_internal)
		{
			if (NULL == _server)
			{
				_server = new TCPServer<>();
				_server->onNewConnection = [&](TCPClient *newClient)
				{
					DEBUG_MSG_FAUXMO("[FAUXMO] New client: [%s:%d]\n",
									 newClient->remoteAddress().c_str(),
									 newClient->remotePort());

					newClient->onMessageReceived = [this, newClient](String message)
					{
						DEBUG_MSG_FAUXMO("[FAUXMO] %s:%d => %s\n",
										 newClient->remoteAddress().c_str(),
										 newClient->remotePort(),
										 message.c_str());
						_onTCPData(newClient, (void *)message.c_str(), message.length());
					};

					newClient->onSocketClosed = [newClient](int errorCode)
					{
						DEBUG_MSG_FAUXMO("[FAUXMO] Socket closed: %s:%d => %d\n",
										 newClient->remoteAddress().c_str(),
										 newClient->remotePort(),
										 errorCode);
						newClient->Close();
					};
				};
				_server->Bind(_tcp_port);
			}
			_server->Listen([](int errorCode, std::string errorMessage) {
				DEBUG_MSG_FAUXMO("[FAUXMO] %s => %d\n", errorMessage.c_str(),
				errorCode);
			});
			DEBUG_MSG_FAUXMO("[FAUXMO] TCP server started\n");
		}

		// UDP setup
		if (NULL == _udp)
		{
			_udp = new UDPServer<>();
			_udp->onRawMessageReceived = [&](const char *message, int length, String ipv4, uint16_t port)
			{
				_onUDPData(message, length, ipv4, port);
			};
			_udp->Bind(FAUXMO_UDP_MULTICAST_PORT, FAUXMO_UDP_MULTICAST_IP.toString().c_str(), [](int errorCode, std::string errorMessage) {
				DEBUG_MSG_FAUXMO("[FAUXMO] %s => %d\n", errorMessage.c_str(),
				errorCode);
			});
			DEBUG_MSG_FAUXMO("[FAUXMO] UDP server started\n");
		}
	}
}

volatile sig_atomic_t interrupt_flag = 0;

int main(int argc, char *argv[])
{
	using namespace std::placeholders;

	// Bind fauxmoLINUX to a network interface e.g. eth0, wlan0,...
	// wlan1 is an USB Wifi dongle, which supports both 2.4GHz and 5GHz
	fauxmoLINUX fauxmo("wlan1");

	// Attach the remote control to a serial port. /dev/ttyS1 is the 4-pin
	// connector at the NanoPi R1, which provides Tx, Rx, Gnd and 5V. Because
	// the application runs under the default user, the permissions of the
	// interface have to changed:
	// Set the permissions of /dev/ttyS1 in /etc/rc.local
	// chown pi /dev/ttyS1
	// chgrp tty /dev/ttyS1
	// Also note that the TTL levels are 3.3V at the NanoPi and 5V at the IR module,
	// therefore a level shifter is needed.
	RemoteControl remote_control("/dev/ttyS1", 9600);

	// If a speech command from Alexa is received, the function actionPerformed
	// determines which infrared byte sequence has to be sent over the serial port.
	fauxmo.onSetState(std::bind(&RemoteControl::actionPerformed, &remote_control, _1, _2, _3, _4));

	// Listen to Ctrl-C to terminate the application
	signal(SIGINT, [](int signum)
		   { interrupt_flag = 1; });

	// By default, fauxmoLINUX creates it's own webserver on the defined port
	// The TCP port must be 80 for gen3 devices (default is 1901)
	// This has to be done before the call to enable()
	fauxmo.createServer(true); // not needed, this is the default value

	// sudo setcap 'cap_net_bind_service=+ep' /home/pi/fauxmoLINUX/build/fauxmoLINUX
	// if port is a privileged port < 1024
	fauxmo.setPort(80); // This is required for gen3 devices

	// You have to call enable(true) once you have a WiFi connection
	// You can enable or disable the library at any moment
	// Disabling it will prevent the devices from being discovered and switched
	fauxmo.enable(true);

	// You can use different ways to invoke alexa to modify the devices state:
	// "Alexa, turn tv on/off"
	// "Alexa, turn on tv"
	// "Alexa, set tv to fifty"

	// Add virtual devices (the name fernseher is the German word for tv)
	fauxmo.addDevice("fernseher");

	// run until Ctrl-C is hit
	while (!interrupt_flag)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}
