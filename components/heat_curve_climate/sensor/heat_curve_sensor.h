#pragma once

#include "../heat_curve_climate.h"
#include "esphome/core/component.h"

namespace esphome {
namespace climate {
namespace heat_curve {

enum HeatCurveClimateSensorType {
  PID_SENSOR_TYPE_RESULT,
  PID_SENSOR_TYPE_SETPOINT,
  PID_SENSOR_TYPE_WATERTEMP,
  PID_SENSOR_TYPE_ERROR,
  PID_SENSOR_TYPE_DELTA,
  PID_SENSOR_TYPE_PROPORTIONAL,
  PID_SENSOR_TYPE_INTEGRAL,
  PID_SENSOR_TYPE_SLOPE,
  PID_SENSOR_TYPE_SHIFT,
  PID_SENSOR_TYPE_KP,
  PID_SENSOR_TYPE_KI
};

class HeatCurveClimateSensor : public sensor::Sensor, public Component, public Parented<HeatingCurveClimate> {
 public:
  void setup() override;
  void set_type(HeatCurveClimateSensorType type) { type_ = type; }
  void dump_config() override;

 protected:
  void update_from_parent_();
  HeatCurveClimateSensorType type_;
};

}  // namespace heat_curve
}  // namespace climate
}  // namespace esphome
