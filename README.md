# Tello RC
Project to control Tello drone using FlySKY Radio Controller instead of your cellphone. It uses Arduino Mega + ESP8266 and a FlySKY receiver compatible with iBus. It also provides the battery feedback on your RC transmitter under the form of a sensor.

![Tello, FlySky transmitter and Arduino board](/page/project.jpg)

## Why am I using an Arduino ATmega2560+ESP8266?
This project requires 2 serial ports: one to receive data from the receiver (the channels) and another to send the battery status to the transmitter (the RC Control). It also needs wi-fi capabilities to communicate with Tello. 

I found the Arduino ATmega2560+ESP8266 board a perfect match for the project. It comes with 4 serial UART interfaces and the ESP8266 wi-fi module. The full board operates at 5v, wich is the Arduino standard, instead of the 3v from the ESP8266. It's also inexpensive (https://banggood.app.link/Qb9GWZ8nH6). The drawback is the USB serial communication that uses multiple switches to select which one you want to control. So, the development process requires a lot of physical switches changes.

![Arduino Mega + ESP8266](/page/arduino_mega.png)

## The Project
Tello  exposes an UDP port that receives commands and another port that exposes stats. The ESP8266 module will first connect to the Tello wi-fi and send a `command` message to stabilish the communication. After the `takeoff`, we use the `rc` command to continously control the drone, sending all the 4 channels together (left/right, forward/backward,  up/down and yaw).

The communication between ATmega2560 and ESP8266 module is made by one of the serial ports. I choose to use a text based communication, just to make it easier to send data and debug messages back and forth between the two modules.

![Project schema](/page/project.png)



