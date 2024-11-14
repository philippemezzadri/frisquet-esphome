#include "pair_button.h"

namespace esphome {
namespace frisquet_boiler {

void PairButton::press_action() { this->parent_->pair(); }

}  // namespace frisquet_boiler
}  // namespace esphome
