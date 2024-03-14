#include "heat_curve_climate.h"

#include "esphome/core/log.h"

namespace esphome {
namespace climate {
namespace heat_curve {
static const char *const TAG = "heating_curve.climate";

void HeatingCurveClimate::setup() {
  // on state callback for current temperature
  if (this->current_sensor_) {
    this->current_sensor_->add_on_state_callback([this](float state) {
      if (state != this->current_temperature) {
        this->do_publish_ = roundf(state * 100) != roundf(this->current_temperature * 100);
        this->current_temperature = state;
        this->update();
      }
    });
    this->current_temperature = this->current_sensor_->state;
  } else
    this->current_temperature = NAN;

  // on state callback for outdoor temperature
  if (this->outoor_sensor_) {
    this->outoor_sensor_->add_on_state_callback([this](float state) {
      this->do_publish_ = roundf(state * 100) != roundf(this->outdoor_temp_ * 100);
      this->outdoor_temp_ = state;
      this->update();
    });
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
void HeatingCurveClimate::control(const ClimateCall &call) {
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
ClimateTraits HeatingCurveClimate::traits() {
  auto traits = ClimateTraits();
  traits.set_supports_current_temperature(this->current_sensor_ != nullptr);
  traits.set_supported_modes({CLIMATE_MODE_OFF, CLIMATE_MODE_HEAT});
  traits.set_supports_two_point_target_temperature(false);
  traits.set_supports_action(true);
  return traits;
}

void HeatingCurveClimate::dump_config() {
  LOG_CLIMATE("", "Heating Curve Climate", this);
  ESP_LOGCONFIG(TAG, "  Control Parameters:");
  ESP_LOGCONFIG(TAG, "    slope: %.2f, shift: %.2f, kp: %.2f, ki: %.5f", this->slope_, this->shift_, this->kp_,
                this->ki_);

  if (this->alt_curve_) {
    ESP_LOGCONFIG(TAG, "    Using alternate heating curve.");
  } else {
    ESP_LOGCONFIG(TAG, "    Using standard heating curve.");
  }

  ESP_LOGCONFIG(TAG, "  Output Parameters:");

  if (this->rounded_) {
    ESP_LOGCONFIG(TAG, "    Rounding enabled.");
  } else {
    ESP_LOGCONFIG(TAG, "    Rounding disabled.");
  }

  ESP_LOGCONFIG(TAG, "    maximum_output: %.2f, minimum_output: %.2f", this->maximum_output_, this->minimum_output_);
  ESP_LOGCONFIG(TAG, "    heat_required_output: %.2f", this->heat_required_output_);
  ESP_LOGCONFIG(TAG, "    output_factor: %.2f,  output_offset: %.2f", this->output_calibration_factor_,
                this->output_calibration_offset_);

  this->dump_traits_(TAG);
}

void HeatingCurveClimate::set_heat_required(bool value) {
  this->heat_required_ = value;
  ESP_LOGD(TAG, "Heat required: %s", value ? "true" : "false");
  this->update();
}

void HeatingCurveClimate::update() {
  if (std::isnan(this->outdoor_temp_)) {
    ESP_LOGW(TAG, "Outdoor temperature not available, skipping calculation.");
    return;
  }

  this->dt_ = calculate_relative_time_();

  float new_temp = get_heat_curve_temp();

  // Proportional and Integral correction to accelerate convergence to target
  if (!std::isnan(this->current_temperature) && !std::isnan(this->target_temperature)) {
    this->error_ = this->target_temperature - this->current_temperature;
    ESP_LOGD(TAG, "Error: %.1f", this->error_);

    this->calculate_proportional_term_();
    this->calculate_integral_term_();
    new_temp += this->proportional_term_ + this->integral_term_;
    ESP_LOGD(TAG, "Adjusted temperature: %.1f°C", new_temp);
  }

  // convert to output
  float output = this->temperature_to_output(new_temp);
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

  ESP_LOGD(TAG, "Output: %.1f%%", output * 100);

  // Recalculate actual water temperature to take into accound rounding
  this->water_temp_ = this->output_to_temperature(output);

  if (this->mode == CLIMATE_MODE_OFF) {
    this->write_output_(0.0);
  } else {
    this->write_output_(output);
  }

  if (this->do_publish_)
    this->publish_state();
}

void HeatingCurveClimate::write_output_(float value) {
  this->output_value_ = value;
  this->output_->set_level(clamp(value, 0.0f, 1.0f));

  // Update action variable for user feedback what's happening
  ClimateAction new_action;
  if (value > 0) {
    new_action = CLIMATE_ACTION_HEATING;
    ESP_LOGI(TAG, "Climate action is HEATING");
  } else if (this->mode == CLIMATE_MODE_OFF) {
    new_action = CLIMATE_ACTION_OFF;
    this->water_temp_ = OFF_MODE_TEMPERATURE;
    ESP_LOGI(TAG, "Climate mode is OFF");
  } else {
    new_action = CLIMATE_ACTION_IDLE;
    this->water_temp_ = OFF_MODE_TEMPERATURE;
    ESP_LOGI(TAG, "Climate action is IDLE");
  }

  if (new_action != this->action) {
    this->action = new_action;
    this->do_publish_ = true;
  }
  this->internal_sensor_callback_.call();
}

float HeatingCurveClimate::calculate_relative_time_() {
  uint32_t now = millis();
  uint32_t dt = now - this->last_time_;
  if (last_time_ == 0) {
    this->last_time_ = now;
    return 0.0f;
  }
  this->last_time_ = now;
  return dt / 1000.0f;
}

bool HeatingCurveClimate::in_deadband() {
  float err = -this->error_;
  return (THRESHOLD_LOW < err && err < THRESHOLD_HIGH);
}

void HeatingCurveClimate::calculate_proportional_term_() {
  // p(t) := K_p * e(t)
  this->proportional_term_ = this->kp_ * this->error_;

  // set dead-zone to -X to +X
  if (in_deadband()) {
    // shallow the proportional_term in the deadband by the pdm
    this->proportional_term_ *= KP_MULTIPLIER;

  } else {
    // pdm_offset prevents a jump when leaving the deadband
    float threshold = (error_ < 0) ? THRESHOLD_HIGH : THRESHOLD_LOW;
    float pdm_offset = (threshold - (KP_MULTIPLIER * threshold)) * kp_;
    this->proportional_term_ += pdm_offset;
  }

  ESP_LOGD(TAG, "Proportionnal term: %.2f", this->proportional_term_);
}

void HeatingCurveClimate::calculate_integral_term_() {
  if (this->ki_ > 0) {
    // i(t) := K_i * \int_{0}^{t} e(t) dt
    float new_integral = this->error_ * this->dt_ * this->ki_;

    // Preventing integral wind up
    if (this->output_value_ <= (this->minimum_output_ / 100.0) && new_integral < 0) {
      return;
    }
    if (this->output_value_ >= (this->maximum_output_ / 100.0) && new_integral > 0) {
      return;
    }

    if (!in_deadband()) {
      this->integral_term_ += new_integral;
    }

    ESP_LOGD(TAG, "Integral term: %.5f", this->integral_term_);
  }
}

float HeatingCurveClimate::output_to_temperature(float output) {
  return (100.0 * output - this->output_calibration_offset_) / this->output_calibration_factor_;
}

float HeatingCurveClimate::temperature_to_output(float temp) {
  float output = temp * this->output_calibration_factor_ + this->output_calibration_offset_;
  return this->rounded_ ? round(output) / 100.0 : output / 100.0;
}

float HeatingCurveClimate::get_heat_curve_temp() {
  this->delta_ = this->target_temperature - this->outdoor_temp_;
  ESP_LOGD(TAG, "Delta T: %.1fK", this->delta_);

  float flow_temp;
  if (this->alt_curve_) {
    ESP_LOGD(TAG, "Using alternate heating curve");
    float delta = -this->delta_;
    flow_temp = this->target_temperature + this->shift_ -
                this->slope_ * delta * (1.4347 + 0.021 * delta + 247.9 * 0.000001 * delta * delta);
  } else {
    flow_temp = this->delta_ * this->slope_ + this->target_temperature + this->shift_;
  }

  ESP_LOGD(TAG, "Heating curve temperature: %.1f°C", flow_temp);
  return flow_temp;
}

}  // namespace heat_curve
}  // namespace climate
}  // namespace esphome
