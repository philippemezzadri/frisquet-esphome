#pragma once

#include "../heat_curve_climate.h"
#include "esphome/core/component.h"

namespace esphome {
namespace climate {
namespace heat_curve {

enum HeatCurveClimateSensorType {
  HEAT_CURVE_SENSOR_TYPE_RESULT,
  HEAT_CURVE_SENSOR_TYPE_SETPOINT,
  HEAT_CURVE_SENSOR_TYPE_WATERTEMP,
  HEAT_CURVE_SENSOR_TYPE_ERROR,
  HEAT_CURVE_SENSOR_TYPE_DELTA,
  HEAT_CURVE_SENSOR_TYPE_PROPORTIONAL,
  HEAT_CURVE_SENSOR_TYPE_INTEGRAL,
  HEAT_CURVE_SENSOR_TYPE_SLOPE,
  HEAT_CURVE_SENSOR_TYPE_SHIFT,
  HEAT_CURVE_SENSOR_TYPE_KP,
  HEAT_CURVE_SENSOR_TYPE_KI
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
