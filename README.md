# Frisquet ESPHome

This custom component allows communication between an ESPHome device
(ESP8266 or ESP32) and a Frisquet heating boiler (equipped with Eco Radio System thermostat).

The ESPHome replaces the original Eco Radio System HF receiver.

Wiring :

| ESP32             | BOILER              |
| ----------------- | ------------------- |
| 21 (configurable) | yellow wire         |
| GND               | black wire          |
| 5V                | red wire (optional) |

This ESPHome Custom Component is strongly integrated with Home Assistant where it appears as a standard climate device. However, if necessary, it can be used with any other home automation system through MQTT. In that case MQTT functionnality should be enabled in the the yaml file.

Refer to:

- https://esphome.io/components/climate/index.html
- https://esphome.io/components/climate/custom.html
- https://esphome.io/components/climate/pid.html

This work is strongly inspired from:

- https://antoinegrall.wordpress.com/decodage-frisquet-ers/
- http://wiki.kainhofer.com/hardware/vaillantvrt340f
- https://github.com/etimou/frisquet-arduino

and from the discussions made in this thread:

- https://easydomoticz.com/forum/viewtopic.php?f=17&t=1486sid=d2f41ac68e5bab18fd412a192a21c2c4
