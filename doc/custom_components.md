# Custom components installation (deprecated)

**Note:** this version of the custom components is deprecated and will not be updated.
It is recommended to use the new externat components.


## Installation

The Frisquet ESPHome component concists in two custom components:

- `HeatCurveClimate.h` a custom `Climate` component that will control the boiler water setpoint based on external temperature measurement and ambiant temperature setpoint.
- `FrisquetBoilerFloatOutput.h` a custom `Output` component that will actually communicate with the Frisquet boiler.

Both files `HeatCurveClimate.h` and `FrisquetBoilerFloatOutput.h` must be copied in your a folder name `custom`in the `esphome` configuration folder.

Your `yaml` configuration file must show at minimum the following code:

```yaml
esphome:
  name: myFrisquetBoiler

  # Installation of the two custom components
  includes:
  - custom/FrisquetBoilerFloatOutput.h
  - custom/HeatCurveClimate.h

globals:
  - id: global_heat_factor
    type: float
    restore_value: yes
    initial_value: "1.7"
  - id: global_offset
    type: float
    restore_value: yes
    initial_value: "20.0"
  - id: global_kp
    type: float
    restore_value: no
    initial_value: "0.0"

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
      custom_climate->set_output_conversion_factor(1.90);
      custom_climate->set_output_conversion_offset(-41);
      App.register_component(custom_climate);
      return {custom_climate};

    climates:
      - name: "${name}"
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
