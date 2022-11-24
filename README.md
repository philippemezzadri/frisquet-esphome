# Frisquet Boiler for ESPHome

This custom component allows communication between an ESPHome device
(ESP8266 or ESP32) and a Frisquet heating boiler (equipped with Eco Radio System remote thermostat).

This ESPHome Custom Component is strongly integrated with **Home Assistant** where it appears as a standard climate device. However, if necessary, it can be used with any other home automation system through [MQTT](<https://esphome.io/components/mqtt.html>). In that case MQTT functionnality should be enabled in the the `yaml` configuration file.

## References

- <https://esphome.io/components/climate/index.html>
- <https://esphome.io/components/climate/custom.html>
- <https://esphome.io/components/output/custom.html>

This work is strongly inspired from:

- <https://antoinegrall.wordpress.com/decodage-frisquet-ers/>
- <http://wiki.kainhofer.com/hardware/vaillantvrt340f>
- <https://github.com/etimou/frisquet-arduino>

and from the discussions made in this thread:

- <https://easydomoticz.com/forum/viewtopic.php?f=17&t=1486sid=d2f41ac68e5bab18fd412a192a21c2c4> (French)

## Wiring

The ESPHome replaces the original Eco Radio System HF receiver and is conneted to the boiler main board through a micro-fit 4 socket.

| ESP32                 | Boiler Side         | Pin number |
| --------------------- | ------------------- |:----------:|
| GND                   | black wire          | 1          |
| Pin 21 (configurable) | yellow wire         | 2          |
| 5V                    | red wire (optional) | 3          |

**Micro-fit 4 pin out:**

<img src="doc/connector_4pin1.png" alt="Micro-fit 4 pinout drawing" width="80"/>

Defined viewing direction for the connector pin out:

- Receptable - _rear view_
- Header - _front view_

<ins>Note</ins>: It has been observed that the current supplied by the boiler main board is not sufficent to power the ESP32.

## Installation

**Note:** for the previous installation method (deprecated) based on custom components, see [here](doc/custom_components.md).

The Frisquet ESPHome component concists in two components:

- `heat_curve_climate` a `climate` component that will control the boiler water setpoint based on external temperature measurement and ambiant temperature setpoint. 
- `friquet_boiler` a `output` component that will actually communicate with the Frisquet boiler.

The complete folder `components` must be copied in your `esphome` configuration folder. 

Then the file `components/frisquet_boiler/frisquet_boiler.cpp` must be edited to set the ID of your boiler:

  ```cpp
  // Set boiler id
  this->message_[4] = 0x03;
  this->message_[5] = 0xB9;
  ```


Your `yaml` configuration file must show at minimum the following code:

```yaml
esphome:
  name: myFrisquetBoiler

external_components:
  - source: components

output:
  - platform: frisquet_boiler
    id: boiler_cmd
    max_power: 1.0
    min_power: 0
    zero_means_zero: true

sensor:
  - platform: homeassistant
    id: current_temperature
    entity_id: sensor.aqara_salon_temperature
    unit_of_measurement: "°C"
    filters:
      - filter_out: nan
      - heartbeat: 60s
        
  - platform: homeassistant
    id: outdoor_temperature
    entity_id: sensor.outdoor_temperature
    unit_of_measurement: "°C"
    filters:
      - filter_out: nan
      - heartbeat: 60s

  - platform: heat_curve_climate
    name: "Consigne chaudière"
    type: WATERTEMP

climate:
  - platform: heat_curve_climate
    id: boiler_climate
    name: "Chaudière Frisquet"
    sensor: current_temperature
    outdoor_sensor: outdoor_temperature
    output: boiler_cmd
    control_parameters:
      output_factor: 1.9
      output_offset: -41
      heat_factor: 1.7
      offset: 20
      kp: 0
```

## Tuning

1. **Heat curve definition**

    The boiler water temperature is calculated from the outdoor temperature:

    `WaterReturnTemperature = (TargetTemp - OutdoorTemp) * HeatFactor + Offset`

    `HeatFactor` and `Offset` are defined as globals in the yaml configuration file.

    Those two parameters strongly depend on the heat insulation of the house. Therefore slight adjustments may be necessary to find the best settings. Guidelines to do so can be found [here](https://blog.elyotherm.fr/2013/08/reglage-optimisation-courbe-de-chauffe.html) (French).

    In order to fine ease the fine tuning of those parameters, a service is available in HA to change the parameters without restarting ESPHome.


2. **Boiler setpoint conversion factor and offset**

    The boiler setpoint (integer in the `[0 - 100]` range) and the water return temperature are linked by the following formula:

    `Setpoint = WaterReturnTemperature * ConversionFactor + Offset`

    `ConversionFactor` and `Offset` are defined using the following lines in the yaml configuration file:

    ```yaml
    control_parameters:
      output_factor: 1.9
      output_offset: -41
    ```
