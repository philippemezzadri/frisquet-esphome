# Frisquet Boiler for ESPHome

This ESPHome component allows communication between an ESPHome device
(ESP8266 or ESP32) and a Frisquet heating boiler (equipped with Eco Radio System remote thermostat).

It is strongly integrated with **Home Assistant** where it appears as a [Float Output](<https://esphome.io/components/output/>) device. However, if necessary, it can be used with any other home automation system through [MQTT](<https://esphome.io/components/mqtt.html>). In that case MQTT functionnality should be enabled in the the `yaml` configuration file.

It is recommended to combine the [Output](<https://esphome.io/components/output/>) component with the [Climate](<https://esphome.io/components/climate/index.html>) component also provided in this project. This Climate component will offer temperature control using an outdoor temperature sensor. If prefered, it is also possible to use an other type of Climate component, such as the [PID Climate](https://esphome.io/components/climate/pid.html?highlight=pid).

## References

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
    pin: 5
    boiler_id: 03B9
    max_power: 1.0
    min_power: 0
    zero_means_zero: true
```

Configuration variables:

- id (Required, [ID](<https://esphome.io/guides/configuration-types.html#config-id>)): The id to use for this output component.
- pin (Required, [Pin Schema](<https://esphome.io/guides/configuration-types.html#config-pin-schema>)): The pin number connnected to the boiler
- boiler_id (Required, string): The identifier of your boiler (see below).
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

- sensor (Required, [ID](<https://esphome.io/guides/configuration-types.html#config-id>)): The sensor that is used to measure the current temperature.
- output (Required, [ID](<https://esphome.io/guides/configuration-types.html#config-id>)): The ID of a float output that increases the current temperature.
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

## Actions

The `frisquet_boiler` [Output](<https://esphome.io/components/output/>) component inherits actions from Float Output and in particular:

### `output.set_level` Action

This action sets the float output to the given level when executed. This can be usefull to set the boiler output if it is not connected to a Climate component.

```yaml
on_...:
  then:
    - output.set_level:
        id: boiler_cmd
        level: 50%
```

_Note_:
This action can also be expressed in [lambdas](<https://esphome.io/guides/automations.html#config-lambda>):

```cpp
// range is 0.0 (off) to 1.0 (on)
id(boiler_cmd).set_level(0.5);
```

## Integration with Home Assistant

### Heat Curve Climate Component

The `heat_curve_climate` [Climate](<https://esphome.io/components/climate/index.html>) component exposes one [service](https://www.home-assistant.io/docs/scripts/service-calls/) to Home Assistant allowing to change the `control_parameters` without restarting the ESPHome device.

```yaml
service: esphome.boiler_send_new_heat_curve
data:
  heat_factor: 1.6
  offset: 22
  kp: 0
```

This sets new values for the control parameters. This can be used to manually tune the controller. Make sure to update the values you want on the YAML file! They will reset on the next reboot.

### Frisquet Boiler Output Component

The `frisquet_boiler` [Output](<https://esphome.io/components/output/>) component exposes one [service](https://www.home-assistant.io/docs/scripts/service-calls/) to Home Assistant:

```yaml
service: esphome.boiler_send_operating_mode
data:
  mode: 3
```

This sets the boiler operating mode ( 0 = eco / 3 = confort / 4 = away).
This parameter is actually included in the frames sent to the boiler but I haven't seen any significant effect of the setting.

### Native API Component

When using the native [API](<https://esphome.io/components/api.html>) with Home Assistant, it is also possible to get data from Home Assistant to ESPHome with user-defined services. When you declare services in your ESPHome YAML file, they will automatically show up in Home Assistant and you can call them directly.

This way it is possible to call the Actions provided by the Boiler Output component:

```yaml
# Example configuration entry
api:
  services:
    - service: set_boiler_setpoint
      variables:
        setpoint: int
      then:
        - output.set_level:
            id: boiler_cmd
            level: !lambda 'return setpoint / 100.0;'
```
