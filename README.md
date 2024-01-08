# 16 Button ESP32 Keypad MQTT Remote

A personal project to send arbitrary signals to any device with a wireless battery-powered 16 button keypad and an
ESP32.

![construct finished product](setup/construct-finished-product.jpg)

## Building the project yourself

(just don't actually)

You can find instructions on how to set up and build the project yourself in the following files:

- [General project setup using Platform IO and optionally CLion](setup/setup.md)
- [Set up your Raspberry Pi with NodeRed and mosquitto](setup/raspi-setup.md)
- [Build/Print the 3D casing](setup/casing-3d.md)

If you end up building this, feel free to let me know!
(don't complain about some of the issues though, there are a few that I can't be bothered to fix anymore)

#### Flaws with this design:

- See the 3D printing section for flaws with the model.
- The battery consumption of the ESP32 is too high, it drains the three AA batteries in just a few days.
  I therefore added a switch afterward to fully turn off the device by cutting the power off.
