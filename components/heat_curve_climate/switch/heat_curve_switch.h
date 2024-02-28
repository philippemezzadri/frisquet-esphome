#pragma once

#include "../heat_curve_climate.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/component.h"

namespace esphome {
namespace climate {
namespace heat_curve {

class HeatCurveClimateSwitch : public switch_::Switch, public Component {
 public:
  void set_parent(HeatingCurveClimate *parent) { parent_ = parent; }
  void dump_config() override;

 protected:
  void write_state(bool state) override;
  HeatingCurveClimate *parent_;
};

}  // namespace heat_curve
}  // namespace climate
}  // namespace esphome
