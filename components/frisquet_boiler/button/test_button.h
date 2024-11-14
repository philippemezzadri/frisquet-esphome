#pragma once

#include "esphome/components/button/button.h"
#include "../frisquet_boiler.h"

namespace esphome {
namespace frisquet_boiler {

class TestButton : public button::Button, public Parented<FrisquetBoiler> {
 public:
  TestButton() = default;

 protected:
  void press_action() override;
};

}  // namespace frisquet_boiler
}  // namespace esphome
