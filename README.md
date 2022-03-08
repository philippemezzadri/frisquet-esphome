# Frisquet Boiler for ESPHome

This custom component allows communication between an ESPHome device
(ESP8266 or ESP32) and a Frisquet heating boiler (equipped with Eco Radio System thermostat).

This ESPHome Custom Component is strongly integrated with **Home Assistant** where it appears as a standard climate device. However, if necessary, it can be used with any other home automation system through [MQTT](<https://esphome.io/components/mqtt.html?highlight=mqtt>). In that case MQTT functionnality should be enabled in the the yaml configuration file.

## References

- <https://esphome.io/components/climate/index.html>
- <https://esphome.io/components/climate/custom.html>
- <https://esphome.io/components/climate/pid.html>

This work is strongly inspired from:

- <https://antoinegrall.wordpress.com/decodage-frisquet-ers/>
- <http://wiki.kainhofer.com/hardware/vaillantvrt340f>
- <https://github.com/etimou/frisquet-arduino>

and from the discussions made in this thread:

- <https://easydomoticz.com/forum/viewtopic.php?f=17&t=1486sid=d2f41ac68e5bab18fd412a192a21c2c4>

## Wiring

The ESPHome replaces the original Eco Radio System HF receiver and is conneted to the boiler main board through a micro-fit 4 socket.

| ESP32                 | BOILER SIDE         | Micro-fit pin      |
| --------------------- | ------------------- |:------------------:|
| GND                   | black wire          | 1                  |
| Pin 21 (configurable) | yellow wire         | 2                  |
| 5V                    | red wire (optional) | 3                  |

**Micro-fit 4 pinout:**

<img src="doc/connector_4pin1.png" alt="Micro-fit 4 pinout drawing" width="80"/>

## Installation

The Frisquet ESPHome component concists in two custom components:

- `HeatCurveClimate.h` a custom `Climate` component that will control the boiler water setpoint based on external temperature measurement and ambiant temperature setpoint. 
- `FrisquetBoilerFloatOutput.h` a custom `Output` component that will actually communicate with the Frisquet boiler.

Both files `HeatCurveClimate.h` and `FrisquetBoilerFloatOutput.h` must be copied in your `esphome` configuration folder.

Your `yaml` configuration file must show at minimum the following code:

```yaml
esphome:
  name: myFrisquetBoiler

  # Installation of the two custom components
  includes:
  - FrisquetBoilerFloatOutput.h
  - HeatCurveClimate.h


# Initialisation of the custom output component
output:
  - platform: custom
    type: float
    lambda: |-
      auto boiler_float_output = new FrisquetBoilerFloatOutput();
      boiler_float_output->set_max_power(1.0);
      boiler_float_output->set_min_power(0);
      boiler_float_output->set_zero_means_zero(true);
      App.register_component(boiler_float_output);
      return {boiler_float_output};
    outputs:
      id: boiler_cmd

# Template sensor to send back the boiler water setpoint to homeassistant
sensor:
  - platform: template
    id: heating_curve
    name: "Consigne chaudière"
    unit_of_measurement: "°C"
    accuracy_decimals: 1

# Initialisation of the custom climate component
# - current_temperature : id of the ambiant temperature sensor
# - outdoor_temperature : id of the external temperature sensor

climate:
  - platform: custom
    id: custom_climate
    lambda: |-
      auto custom_climate = new HeatCurveClimate();
      custom_climate->set_sensor(id(current_temperature));
      custom_climate->set_outdoor_sensor(id(outdoor_temperature));
      custom_climate->set_output(id(boiler_cmd));
      custom_climate->set_water_temp_sensor(id(heating_curve));
      custom_climate->set_heat_factor(1.4);
      custom_climate->set_offset(21);
      custom_climate->set_output_conversion_factor(1.90);
      custom_climate->set_output_conversion_offset(-41);
      App.register_component(custom_climate);
      return {custom_climate};
```

## Temperature sensors

At minimum, two temperature sensors must be defined : ambiant temperature and external (outdoor) temperature.

This can be done using the **Home Assistant** API :

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
```

Alternatively, the **mqtt_subscribe** platform can be used if Home Assistant is not used:

```yaml
sensor:
  - platform: mqtt_subscribe
    id: current_temperature
    topic: the/current_temperature/topic
    unit_of_measurement: "°C"
    filters:
      - filter_out: nan
      - heartbeat: 60s
        
  - platform: mqtt_subscribe
    id: outdoor_temperature
    topic: the/outdoor_temperature/topic
    unit_of_measurement: "°C"
    filters:
      - filter_out: nan
      - heartbeat: 60s
```

## Tuning

1. **Boiler setpoint conversion factor and offset**

    The boiler setpoint (integer in the `[0 - 100]` range) and the water return temperature are linked by the following formula:

    `WaterReturnTemperature =  Setpoint * ConversionFactor + Offset`

    `ConversionFactor`and `Offset` are defined using the following lines in the yaml configuration file:

    ```cpp
    custom_climate->set_output_conversion_factor(1.90);
    custom_climate->set_output_conversion_offset(-41);
    ```

2. **Heat curve definition**

    The boiler water temperature is calculated from the outdoor temperature:

    `BoilerTemperature =  (Setpoint - OutdoorTemp) * HeatFactor + Offset`

    `HeatFactor`and `Offset` are defined using the following lines in the yaml configuration file:

    ```cpp
    custom_climate->set_heat_factor(1.4);
    custom_climate->set_offset(21);
    ```
