# Frisquet Boiler for ESPHome

This custom component allows communication between an ESPHome device
(ESP8266 or ESP32) and a Frisquet heating boiler (equipped with Eco Radio System remote thermostat).

This ESPHome Custom Component is strongly integrated with **Home Assistant** where it appears as a standard climate device. However, if necessary, it can be used with any other home automation system through [MQTT](<https://esphome.io/components/mqtt.html>). In that case MQTT functionnality should be enabled in the the `yaml` configuration file.

## References

- <https://esphome.io/components/external_components.html>
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

_Note_: It has been observed that the current supplied by the boiler main board is not sufficent to power the ESP32.

## Installation

**Note:** for the previous installation method (deprecated) based on custom components, see [here](doc/custom_components.md).

The Frisquet ESPHome component concists in two components:

- `heat_curve_climate` a `climate` component that will control the boiler water setpoint based on external temperature measurement and ambiant temperature setpoint. 
- `friquet_boiler` a `output` component that will actually communicate with the Frisquet boiler.

The complete folder `components` must be copied in your `esphome` configuration folder.

Your `yaml` configuration file must show at minimum the following code:

### External components

```yaml
esphome:
  name: myFrisquetBoiler

external_components:
  - source: components
```

### Output

```yaml
output:
  - platform: frisquet_boiler
    id: boiler_cmd
    boiler_id: 03B9
    max_power: 1.0
    min_power: 0
    zero_means_zero: true
```

Configuration variables:

- id (Required, ID): The id to use for this output component.
- boiler_id (Require, string): The identifier of your boiler (see below).
- All other options from [Float Output](<https://esphome.io/components/output/>)
- `power_supply` and `inverted` from [Output](<https://esphome.io/components/output/>) are _not_ considered.

The output value received by the component is any rational value between 0 and 1 like the [Float Outputs](<https://esphome.io/components/output/>). Internaly, the output value is multiplied by 100 and cast as an integer because the Frisquet Boiler accepts orders as integers between 0 and 100 :

- 0 : boiler is stopped
- 10 : water pump starts, no heating
- 11 - 100 : water heating
- 15 : for some reason, the value is not accepted by the boiler. Internally, 15 is converted to 16 to avoid this case.

**Important:** the boiler ID that must be indicated in the yaml configuration file is required to allow
your boiler to receive the messages from the ESP. It can be retrieved by connecting the radio receiver wire to an Arduino. See [here](https://github.com/etimou/frisquet-arduino) for more details.

### Sensors

```yaml
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
```

The `heat_curve_climate` platform allows you to create optional sensors giving you feedback from the component.

Configuration variables:

- name (Required, string): The name of the sensor
- type (Required, string): The value to monitor. One of
  - RESULT - The resulting value sent to the output component (float between 0 and 1).
  - SETPOINT - The setpoint sent to the boiler (%, actually 100 * RESULT).
  - WATERTEMP - The resulting water temperature resulting from SETPOINT.
  - DELTA - The temperature difference between the target and the outdoor.
  - ERROR - The calculated error (target - process_variable)
  - PROPORTIONAL - The proportional term of the controller (if kp is not 0).

Those sensors may be useful to set up your heat curve parameters.

### Climate

```yaml
climate:
  - platform: heat_curve_climate
    id: boiler_climate
    name: "Chaudière Frisquet"
    sensor: current_temperature
    outdoor_sensor: outdoor_temperature
    output: boiler_cmd
    visual:
      min_temperature: 7
      max_temperature: 28
      temperature_step: 0.1
    control_parameters:
      heat_factor: 1.8
      offset: 20
      kp: 0
    output_parameters:
      minimum_output: 0.1
      output_factor: 1.9
      output_offset: -41
```

Configuration variables:

- sensor (Required, ID): The sensor that is used to measure the current temperature.
- output (Required, ID): The ID of a float output that increases the current temperature.
- control_parameters (Required): Control parameters of the controller.
  - heat_factor (Required, float): The proportional term (slope) of the heat curve.
  - offset (Required, float): The offset term of the heat curve.
  - kp (Optional, float): The factor for the proportional term of the controller. Defaults to 0.
- output_parameters (Optional): Output parameters of the controller.
  - minimum_output (Optional, float): Output value below which output value is set to zero. Defaults to 0.1.
  - output_factor (Optional, float): Calibration factor of the output. Defaults to 1.
  - output_offset (Optional, float): Calibration offset of the output. Defaults to 0.
- All other options from [Climate](<https://esphome.io/components/climate/index.html#config-climate>)

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
    output_parameters:
      output_factor: 1.9
      output_offset: -41
    ```
