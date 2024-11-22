#pragma once

#include "esphome/components/switch/switch.h"
#include "../frisquet_boiler.h"

namespace esphome {
namespace frisquet_boiler {

class PairSwitch : public switch_::Switch, public Parented<FrisquetBoiler> {
 public:
  PairSwitch() = default;

 protected:
  void write_state(bool state) override;
};

}  // namespace frisquet_boiler
}  // namespace esphome
