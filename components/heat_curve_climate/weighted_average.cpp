#include "weighted_average.h"

#include "esphome/core/log.h"

namespace esphome {
namespace climate {
namespace heat_curve {

static const char *const TAG = "heating_curve.climate";
static const float WEIGHTED_AVERAGE_ALPHA = 0.2;

WeightedAverage::WeightedAverage() : alpha_(WEIGHTED_AVERAGE_ALPHA) {}

float WeightedAverage::new_value(float value) {
  if (!std::isnan(value)) {
    if (this->first_value_) {
      this->accumulator_ = value;
      this->first_value_ = false;
    } else {
      this->accumulator_ = (this->alpha_ * value) + (1.0f - this->alpha_) * this->accumulator_;
    }
  }

  const float average = std::isnan(value) ? value : this->accumulator_;
  ESP_LOGD(TAG, "WeightedAverage new_value(%f) -> %f", value, average);

  return average;
}

}  // namespace heat_curve
}  // namespace climate
}  // namespace esphome