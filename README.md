# FauxmoLINUX

Amazon Alexa support for NanoPi R1 https://wiki.friendlyelec.com/wiki/index.php/NanoPi_R1 + YS-IRTM IR module https://server4.eca.ir/eshop/000/other/NEC%20infrared%20codec%20module%20YS-IRTM.pdf. The code is based on the https://github.com/vintlabs/fauxmoESP and ported to a headless FriendlyCore Linux based on a SD card image "h3-sd-friendlycore-focal-4.14-armhf-20250402.img".

[![license](https://img.shields.io/badge/license-MIT-orange.svg)](LICENSE)

## Libraries

- A patched version of `async-sockets-cpp` to support multi-cast https://github.com/eminfedar/async-sockets-cpp
- Boost 1.71.0 (install with `sudo apt-get install libboost-all-dev`)

## Usage

Create an instance of fauxmoLINUX and bind it to an network interface e.g. "eth0, "wlan0", ... FauxmoLINUX emulates a Philips VUE light and does the communication with Alexa. Then create a RemoteControl instance for sending infrared codes over a serial interface and binds its `actionPerformed` callback to the fauxmoLINUX instance, which maps the Alexa commands to an infrared byte sequence. Using an YS-IRTM NEC Infrared Codec Module connected to the serial port, the codes can be sent to an infrared receiver like a tv to switch the device on/off or change to another channel. Supported devices are LG UHD 4K TV 2020 43UM7000PLA 49UM7000PLA 55UM7000PLC 65UM7000PLA 75UM7000PLA 43UM7100PLB 49UM7100PLB 55UM7100PLB 60UM7100PLB 65UM7100PLA 70UM7100PLA AKB75875301.

```
	fauxmoLINUX fauxmo("wlan1");
	RemoteControl remote_control("/dev/ttyS1", 9600);
	fauxmo.onSetState(std::bind(&RemoteControl::actionPerformed, &remote_control, _1, _2, _3, _4));
```
## Setup

The executable fauxmoLINUX opens a UDP port 80 as a default user `pi`. To allow this, the following setup is necessary.
```
sudo setcap 'cap_net_bind_service=+ep' /home/pi/fauxmoLINUX/build/fauxmoLINUX
```
The permissions of the serial interface have to changed by setting the permissions of `/dev/ttyS1` in `/etc/rc.local`:
```
chown pi /dev/ttyS1
chgrp tty /dev/ttyS1
```
Another modification was disabling `g_mass_storage` and enabling `g_serial`. So when NanoPi is connected to a computer over the USB port, a serial interface opens on the host computer.

## License

The MIT License (MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
