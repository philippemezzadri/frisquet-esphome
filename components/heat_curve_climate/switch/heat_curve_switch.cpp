#include "heat_curve_switch.h"

#include "esphome/core/log.h"

namespace esphome {
namespace climate {
namespace heat_curve {

static const char *const TAG = "heating_curve.switch";

void HeatCurveClimateSwitch::dump_config() { LOG_SWITCH("", "Heating Curve Climate Switch", this); }

void HeatCurveClimateSwitch::write_state(bool state) {
  this->parent_->set_heat_required(state);
  this->publish_state(state);
}

}  // namespace heat_curve
}  // namespace climate
}  // namespace esphome
