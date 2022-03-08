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

# Template sensor to send back the boiler water setpoit to homeassistant
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

## Tuning

1. **Boiler setpoint conversion factor and offset**

    The boiler setpoint (integer in 0-100 range) and the water return temperature are linked by the following formula:

    `WaterReturnTemperature =  Setpoint * ConversionFactor + Offset`

    This is made using the following commands:

    ```cpp
    custom_climate->set_output_conversion_factor(1.90);
    custom_climate->set_output_conversion_offset(-41);
    ```

2. **Heat curve definition**




