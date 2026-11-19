#pragma once

#include "../heat_curve_climate.h"
#include "esphome/components/number/number.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

namespace esphome {
namespace climate {
namespace heat_curve {

enum HeatCurveClimateNumberType {
  HEAT_CURVE_NUMBER_TYPE_SLOPE,
  HEAT_CURVE_NUMBER_TYPE_SHIFT,
  HEAT_CURVE_NUMBER_TYPE_KP,
  HEAT_CURVE_NUMBER_TYPE_KI,
  HEAT_CURVE_NUMBER_TYPE_MAX_ERROR,
  HEAT_CURVE_NUMBER_TYPE_MIN_DELTA,
};

class HeatCurveClimateNumber : public number::Number, public Component, public Parented<HeatingCurveClimate> {
 public:
  void setup() override;
  void dump_config() override;

  void set_type(HeatCurveClimateNumberType type) { type_ = type; }
  void set_restore_value(bool restore_value) { restore_value_ = restore_value; }

 protected:
  void control(float value) override;

  float read_parameter_();
  void write_parameter_(float value);
  void save_(float value);

  HeatCurveClimateNumberType type_;
  bool restore_value_{true};
  ESPPreferenceObject pref_;
};

}  // namespace heat_curve
}  // namespace climate
}  // namespace esphome
