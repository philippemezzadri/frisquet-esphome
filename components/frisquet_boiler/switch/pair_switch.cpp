#include "pair_switch.h"

namespace esphome {
namespace frisquet_boiler {

void PairSwitch::write_state(bool state) {
  this->parent_->set_mode(state ? CONFIG_MODE : CONTROL_MODE);
  this->publish_state(state);
}

}  // namespace frisquet_boiler
}  // namespace esphome
