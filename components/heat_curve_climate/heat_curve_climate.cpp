#include "heat_curve_climate.h"

#include "esphome/core/log.h"

namespace esphome {
namespace climate {
namespace heat_curve {
static const char *const TAG = "heat_curve.climate";

static const float THRESHOLD_HIGH = 0.15;
static const float THRESHOLD_LOW = -0.15;

void HeatCurveClimate::setup() {
  // on state callback for current temperature
  if (this->current_sensor_) {
    this->current_sensor_->add_on_state_callback([this](float state) {
      if (state != this->current_temperature)
      {
        this->do_publish_ = roundf(state * 100) != roundf(this->current_temperature * 100);
        this->current_temperature = state;
        this->update();        
      } });
    this->current_temperature = this->current_sensor_->state;
  } else
    this->current_temperature = NAN;

  // on state callback for outdoor temperature
  if (this->outoor_sensor_) {
    this->outoor_sensor_->add_on_state_callback([this](float state) {
      this->do_publish_ = roundf(state * 100) != roundf(this->outdoor_temp_ * 100);
      this->outdoor_temp_ = state;
      this->update(); });
    this->outdoor_temp_ = this->outoor_sensor_->state;
  } else
    this->outdoor_temp_ = 15.0;  // Default outdoor temp if there is no sensor

  // restore set points
  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->apply(this);
  } else {
    // restore from defaults
    this->mode = CLIMATE_MODE_HEAT;
    this->target_temperature = this->default_target_temperature_;
  }
}

/// Override control to change settings of the climate device
void HeatCurveClimate::control(const ClimateCall &call) {
  if (call.get_mode().has_value())
    this->mode = *call.get_mode();
  if (call.get_target_temperature().has_value())
    this->target_temperature = *call.get_target_temperature();

  // If switching to off mode, set output immediately
  if (this->mode == CLIMATE_MODE_OFF)
    this->write_output_(0.0f);

  this->publish_state();
}

/// Return the traits of this controller
ClimateTraits HeatCurveClimate::traits() {
  auto traits = ClimateTraits();
  traits.set_supports_current_temperature(this->current_sensor_ != nullptr);
  traits.set_supported_modes({CLIMATE_MODE_OFF, CLIMATE_MODE_HEAT});
  traits.set_supports_two_point_target_temperature(false);
  traits.set_supports_action(true);
  return traits;
}

void HeatCurveClimate::dump_config() {
  LOG_CLIMATE("", "Heat Curve Climate", this);
  ESP_LOGCONFIG(TAG, "  Control Parameters:");
  ESP_LOGCONFIG(TAG, "    heat_factor: %.2f", this->heat_factor_);
  ESP_LOGCONFIG(TAG, "    offset: %.2f", this->offset_);
  ESP_LOGCONFIG(TAG, "    kp: %.2f", this->kp_);
  ESP_LOGCONFIG(TAG, "    ki: %.5f", this->ki_);
  ESP_LOGCONFIG(TAG, "  Output Parameters:");
  ESP_LOGCONFIG(TAG, "    maximum_output_: %.2f", this->maximum_output_ / 100.0);
  ESP_LOGCONFIG(TAG, "    minimum_output_: %.2f", this->minimum_output_ / 100.0);
  ESP_LOGCONFIG(TAG, "    output_factor: %.2f", this->output_calibration_factor_);
  ESP_LOGCONFIG(TAG, "    output_offset: %.2f", this->output_calibration_offset_);
  this->dump_traits_(TAG);
}

void HeatCurveClimate::set_heat_required(bool value) {
  this->heat_required_ = value;
  ESP_LOGD(TAG, "Heat required: %s", value ? "true" : "false");
  this->update();
}

void HeatCurveClimate::update() {
  float new_temp;
  float output;

  if (std::isnan(this->outdoor_temp_)) {
    ESP_LOGD(TAG, "Outdoor temperature not available, skipping calculation.");
    return;
  }

  dt_ = calculate_relative_time_();

  // New return water temperature according to heat curve
  this->delta_ = this->target_temperature - this->outdoor_temp_;

  // water_temp(delta_t) := heat_factor * delta_t + offset
  new_temp = this->delta_ * this->heat_factor_ + this->offset_;

  ESP_LOGD(TAG, "Delta T: %.1f", this->delta_);
  ESP_LOGD(TAG, "Heating curve temperature: %.1f°C", new_temp);

  // Proportional and Integral correction to accelerate convergence to target
  if (!std::isnan(this->current_temperature) && !std::isnan(this->target_temperature)) {
    this->error_ = this->target_temperature - this->current_temperature;
    ESP_LOGD(TAG, "Error: %.1f", this->error_);

    this->calculate_proportional_term_();
    this->calculate_integral_term_();

    new_temp += this->proportional_term_ + this->integral_term_;
  }

  ESP_LOGD(TAG, "Calculated temperature: %.1f°C", new_temp);

  // Boiler setpoint calculation according to water temperature and calibration factors
  output = floor(new_temp * this->output_calibration_factor_ + this->output_calibration_offset_ + 0.5);
  output = clamp(output, 0.0f, this->maximum_output_);

  // shutdown boiler if outdoor temperature is too high or output below minimum value
  if (this->outdoor_temp_ > this->target_temperature - 2 || output < this->minimum_output_) {
    ESP_LOGD(TAG, "Forcing IDLE");
    output = 0;
  }

  // if heat required by switch, minimum output is heat_required_output_
  if (this->heat_required_ && output < this->heat_required_output_) {
    ESP_LOGD(TAG, "Forcing heat required minimum output");
    output = this->heat_required_output_;
  }

  // Recalculate actual water temperature (knowing that the output is an integer)
  new_temp = (output - this->output_calibration_offset_) / this->output_calibration_factor_;
  ESP_LOGD(TAG, "Calculated output: %.0f%%", output);
  ESP_LOGD(TAG, "Corrected temperature: %.1f°C", new_temp);
  this->water_temp_ = new_temp;

  if (this->mode == CLIMATE_MODE_OFF) {
    this->write_output_(0.0);
  } else {
    this->write_output_(output / 100);
  }

  if (this->do_publish_)
    this->publish_state();
}

void HeatCurveClimate::write_output_(float value) {
  this->output_value_ = value;
  this->output_->set_level(clamp(value, 0.0f, 1.0f));

  // Update action variable for user feedback what's happening
  ClimateAction new_action;
  if (value > 0) {
    new_action = CLIMATE_ACTION_HEATING;
    ESP_LOGD(TAG, "Climate action is HEATING");
  } else if (this->mode == CLIMATE_MODE_OFF) {
    new_action = CLIMATE_ACTION_OFF;
    this->water_temp_ = 20;
    ESP_LOGD(TAG, "Climate mode is OFF");
  } else {
    new_action = CLIMATE_ACTION_IDLE;
    this->water_temp_ = 20;
    ESP_LOGD(TAG, "Climate action is IDLE");
  }

  ESP_LOGD(TAG, "Water temperature setpoint: %.1f°C", this->water_temp_);

  if (new_action != this->action) {
    this->action = new_action;
    this->do_publish_ = true;
  }
  this->water_temp_computed_callback_.call();
}

float HeatCurveClimate::calculate_relative_time_() {
  uint32_t now = millis();
  uint32_t dt = now - this->last_time_;
  if (last_time_ == 0) {
    last_time_ = now;
    return 0.0f;
  }
  last_time_ = now;
  return dt / 1000.0f;
}

bool HeatCurveClimate::in_deadband() {
  // return (fabs(error) < deadband_threshold);
  float err = -error_;
  return ((err > 0 && err < THRESHOLD_HIGH) || (err < 0 && err > THRESHOLD_LOW));
}

void HeatCurveClimate::calculate_proportional_term_() {
  // p(t) := K_p * e(t)
  proportional_term_ = kp_ * error_;
  ESP_LOGD(TAG, "Proportionnal term: %.2f", this->proportional_term_);
}

void HeatCurveClimate::calculate_integral_term_() {
  // i(t) := K_i * \int_{0}^{t} e(t) dt
  float new_integral = error_ * dt_ * ki_;

  // Preventing integral wind up
  if (this->output_value_ == 0 && new_integral < 0) {
    return;
  }
  if (this->output_value_ >= 1 && new_integral > 0) {
    return;
  }

  if (!in_deadband()) {
    integral_term_ += new_integral;
  }

  ESP_LOGD(TAG, "Integral term: %.5f", this->integral_term_);
}

}  // namespace heat_curve
}  // namespace climate
}  // namespace esphome
