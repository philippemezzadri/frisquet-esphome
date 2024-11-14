#include "test_button.h"

namespace esphome {
namespace frisquet_boiler {

void TestButton::press_action() { this->parent_->send_test_message(); }

}  // namespace frisquet_boiler
}  // namespace esphome
