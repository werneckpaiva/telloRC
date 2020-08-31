# Tello RC
Project to control Tello drone using FlySKY Radio Controller instead of your cellphone. It uses Arduino Mega + ESP8266 and a FlySKY receiver compatible with iBus. It also provides the battery feedback on your RC transmitter under the form of a sensor telemetry.

![Tello, FlySky transmitter and Arduino board](/page/project.jpg)

## Why am I using an Arduino ATmega2560+ESP8266?
This project requires 2 serial ports: one to receive data from the receiver (the channels) and another to send the battery status to the transmitter (the RC Control). It also needs wi-fi capabilities to communicate with Tello.

I found the Arduino ATmega2560+ESP8266 board a perfect match for the project. It comes with 4 serial UART interfaces and the ESP8266 wi-fi module. The full board operates at 5v, wich is the Arduino standard, instead of the 3v from the ESP8266. It's also inexpensive (https://banggood.app.link/Qb9GWZ8nH6). The drawback is the USB serial communication that uses multiple switches to select which one you want to control. So, the development process requires a lot of physical switches changes.

![Arduino Mega + ESP8266](/page/arduino_mega.png)

## The Project
Tello  exposes an UDP port that receives commands and another port that exposes stats. The ESP8266 module will first connect to the Tello wi-fi and send a `command` message to stabilish the communication. One channel is used as a switch to take-off and to land. After the take-off, we use the `rc` command to continously control the drone, sending all the 4 channels together (left/right, forward/backward, up/down and yaw).

The communication between ATmega2560 and ESP8266 module is made by one of the serial ports. I choose to use a text based communication, just to make it easier to send data and debug messages back and forth between the two modules.

![Project schema](/page/project.png)

## Circuit

The FlySky iBus receiver uses 2 serial communications, one for the channels and another for the telemetry. Honestly, I would prefer a single serial full duplex communication for both data and telemetry, but it's not the case. To make it even more complex, the telemetry uses only one pin on a half-duplex communication, so it requires to 'join' the rx and tx with a resistor. If you want to read more, refer to [this post](https://github.com/betaflight/betaflight/wiki/Single-wire-FlySky-(IBus)-telemetry).

Maybe there is an smarter way to do that using only one serial, but I didn't figure out yet. Feel free to contribute if you know how to improve.

![Circuit schema](/page/schema.png)

## The code

There are two arduino sketches, one for the ATmega2560 unit and another for the ESP8266.

### rc_comm

This sketch is used to communicate with the RC receiver and send the data from channels data to the ESP8266. It continously reads the RC channels from the reciever; then serialize it as a string with the formart `>channelNumber:value`; and finally sends it through the serial interface with the ESP8266. It also reads messages from the serial. They can be debug or data messages. The two types data messages are the battery level and the temperature level. Both of them are sent back the the RC receiver as sensors on the telemetry.

To upload the code to the Arduino Mega + ESP8266 you need to set the switches as (1: on, 2: on, 3: on, 4: on, 5: off, 6: off, 7: off, 8: off).

### tello_comm

This sketch is used to send messages to Tello using the UDP API through port 8889. It basically uses 4 commands:
`command` - to stabilish the communication.
`takeoff` - To take off. It's controlled by the RC transmitter switch on channel 5.
`land` - To land. It's also controlled by the channel 5 switch.
`rc` - Sends the control of each gimbal channel.

It also reads stats from the UDP port 8890. The stats come serialized as a string with several different key and values separated by ';'. This code parses this string, and selects the battery and the temperature.

It reads the RC channels from the serial interface and it writes the stats back.

To upload the code to the Arduino Mega + ESP8266 you need to set the switches as (1: off, 2: off, 3: off, 4: off, 5: on, 6: on, 7: on, 8: off).

## See it working

![Video of RC controller working](/page/tello_rc.gif)

## What to do next?
This solution only controls the Tello drone gimbals. It would be nice to stream the video to some kind of goggles, but as I don't have one, I have no idea on how to do it.

## Conclusion

I tested this solution in an indoor environment. The RC range is much higher than the regular range of your phone wifi, but I don't know the range of the ESP8622 itself. It can be used with an external antenna that may increase the distance. Keep in mind that if you are bringing the arduino with you (while walking for instance) the range to be condifered is the wi-fi provided by the ESP8622 module, not by the RC controller.
