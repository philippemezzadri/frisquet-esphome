#pragma once

#include "../frisquet_boiler.h"
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace frisquet_boiler {

enum FrisquetBoilerSensorType {
  BOILER_SENSOR_TYPE_SETPOINT,
  BOILER_SENSOR_TYPE_FLOWTEMP,
};

class FrisquetBoilerSensor : public sensor::Sensor, public Component, public Parented<FrisquetBoiler> {
 public:
  void setup() override;
  void set_type(FrisquetBoilerSensorType type) { type_ = type; }
  void dump_config() override;

 protected:
  void update_from_parent_();
  FrisquetBoilerSensorType type_;
};

}  // namespace frisquet_boiler
}  // namespace esphome
