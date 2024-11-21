#include "test_switch.h"

namespace esphome {
namespace frisquet_boiler {

void TestSwitch::dump_config() { LOG_SWITCH("", "Frisquet Boiler test switch", this); }

void TestSwitch::write_state(bool state) {
  this->parent_->set_mode(state ? TEST_MODE : CONTROL_MODE);
  this->publish_state(state);
}

}  // namespace frisquet_boiler
}  // namespace esphome
