#pragma once

#include "esphome/core/helpers.h"

namespace esphome {
namespace climate {
namespace heat_curve {

class WeightedAverage {
 public:
  WeightedAverage();
  float new_value(float value);
  float value() { return accumulator_; }

 protected:
  bool first_value_{true};
  float accumulator_{NAN};
  float alpha_;
};

}  // namespace heat_curve
}  // namespace climate
}  // namespace esphome