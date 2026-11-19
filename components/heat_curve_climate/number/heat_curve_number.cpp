#include "heat_curve_number.h"

#include "esphome/core/log.h"

namespace esphome {
namespace climate {
namespace heat_curve {

static const char *const TAG = "heating_curve.number";

void HeatCurveClimateNumber::setup() {
  // Start from the value set in the YAML control_parameters
  float value = this->read_parameter_();

  if (this->restore_value_) {
    // Always stored in flash, so values also survive a power cut on ESP8266
    this->pref_ = this->make_entity_preference<float>(0, true);
    float stored;
    if (this->pref_.load(&stored) && !std::isnan(stored)) {
      value = stored;
      this->write_parameter_(value);
    }
  }
  this->publish_state(value);

  // Keep the entity in sync when parameters are changed by the set_control_parameters action
  this->parent_->add_on_control_parameters_callback([this]() {
    float current = this->read_parameter_();
    if (current != this->state) {
      this->publish_state(current);
      this->save_(current);
    }
  });
}

void HeatCurveClimateNumber::dump_config() {
  LOG_NUMBER("", "Heating Curve Climate Number", this);
  ESP_LOGCONFIG(TAG, "  Restore value: %s", YESNO(this->restore_value_));
}

void HeatCurveClimateNumber::control(float value) {
  this->write_parameter_(value);
  this->publish_state(value);
  this->save_(value);

  // A new heating curve or gain makes the accumulated integral term meaningless
  if (this->type_ == HEAT_CURVE_NUMBER_TYPE_SLOPE || this->type_ == HEAT_CURVE_NUMBER_TYPE_SHIFT ||
      this->type_ == HEAT_CURVE_NUMBER_TYPE_KP) {
    this->parent_->reset_integral_term();
  }
  this->parent_->update();
}

float HeatCurveClimateNumber::read_parameter_() {
  switch (this->type_) {
    case HEAT_CURVE_NUMBER_TYPE_SLOPE:
      return this->parent_->get_slope();
    case HEAT_CURVE_NUMBER_TYPE_SHIFT:
      return this->parent_->get_shift();
    case HEAT_CURVE_NUMBER_TYPE_KP:
      return this->parent_->get_kp();
    case HEAT_CURVE_NUMBER_TYPE_KI:
      return this->parent_->get_ki();
    case HEAT_CURVE_NUMBER_TYPE_MAX_ERROR:
      return this->parent_->get_max_error();
    case HEAT_CURVE_NUMBER_TYPE_MIN_DELTA:
      return this->parent_->get_min_delta();
  }
  return NAN;
}

void HeatCurveClimateNumber::write_parameter_(float value) {
  switch (this->type_) {
    case HEAT_CURVE_NUMBER_TYPE_SLOPE:
      this->parent_->set_slope(value);
      break;
    case HEAT_CURVE_NUMBER_TYPE_SHIFT:
      this->parent_->set_shift(value);
      break;
    case HEAT_CURVE_NUMBER_TYPE_KP:
      this->parent_->set_kp(value);
      break;
    case HEAT_CURVE_NUMBER_TYPE_KI:
      this->parent_->set_ki(value);
      break;
    case HEAT_CURVE_NUMBER_TYPE_MAX_ERROR:
      this->parent_->set_max_error(value);
      break;
    case HEAT_CURVE_NUMBER_TYPE_MIN_DELTA:
      this->parent_->set_min_delta(value);
      break;
  }
}

void HeatCurveClimateNumber::save_(float value) {
  if (this->restore_value_)
    this->pref_.save(&value);
}

}  // namespace heat_curve
}  // namespace climate
}  // namespace esphome
