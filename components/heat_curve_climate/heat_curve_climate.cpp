#include "heat_curve_climate.h"

#include "esphome/core/log.h"

namespace esphome {
namespace climate {
namespace heat_curve {
static const char *const TAG = "heating_curve.climate";

void HeatingCurveClimate::setup() {
  if (this->output_calibration_factor_ == 0.0f) {
    ESP_LOGE(TAG, "setup: output_calibration_factor cannot be zero, disabling component");
    this->mark_failed();
    return;
  }

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
  auto traits = climate::ClimateTraits();
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE | climate::CLIMATE_SUPPORTS_ACTION);
  traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT});
  traits.clear_feature_flags(climate::CLIMATE_REQUIRES_TWO_POINT_TARGET_TEMPERATURE);
  return traits;
}

void HeatingCurveClimate::dump_config() {
  LOG_CLIMATE("", "Heating Curve Climate", this);
  ESP_LOGCONFIG(TAG,
                "  Control Parameters\n"
                "    slope: %.2f, shift: %.2f, kp: %.2f, ki: %.5f\n"
                "    max_error: %.1fK\n"
                "    min_delta: %.1fK\n",
                this->slope_, this->shift_, this->kp_, this->ki_, this->max_error_, this->min_delta_);

  if (this->alt_curve_) {
    ESP_LOGCONFIG(TAG, "    Using alternate heating curve.");
  }

  ESP_LOGCONFIG(TAG,
                "  Output Parameters\n"
                "    maximum_output: %.2f\n"
                "    minimum_output: %.2f\n"
                "    heat_required_output: %.2f\n"
                "    output_factor: %.2f\n"
                "    output_offset: %.2f\n",
                this->maximum_output_, this->minimum_output_, this->heat_required_output_,
                this->output_calibration_factor_, this->output_calibration_offset_);

  if (this->rounded_) {
    ESP_LOGCONFIG(TAG, "    Rounding enabled.");
  }

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

  ESP_LOGD(TAG,
           "Calculation loop started\n"
           "  Current temperature: %.1f°C\n"
           "  Target temperature: %.1f°C\n"
           "  Outdoor temperature: %.1f°C",
           this->current_temperature, this->target_temperature, this->outdoor_temp_);

  this->dt_ = calculate_relative_time_();
  float new_temp = get_heat_curve_temp();

  // Proportional and Integral correction to accelerate convergence to target
  if (!std::isnan(this->current_temperature) && !std::isnan(this->target_temperature)) {
    this->error_ = this->target_temperature - this->current_temperature;
    this->calculate_proportional_term_();
    this->calculate_integral_term_();
    new_temp += this->proportional_term_ + this->integral_term_;

    ESP_LOGD(TAG,
             "  Error: %.1f\n"
             "  Proportional term: %.2fK\n"
             "  Integral term: %.3fK\n"
             "  Adjusted temperature: %.1f°C",
             this->error_, this->proportional_term_, this->integral_term_, new_temp);
  }

  this->water_temp_ = new_temp;

  // convert to output
  float output = this->temperature_to_output(new_temp);
  output = clamp(output, 0.0f, this->maximum_output_);

  if (this->delta_ < this->min_delta_) {
    // shutdown boiler if outdoor temperature is too high
    ESP_LOGI(TAG, "Outdoor temperature above max limit, forcing IDLE");
    output = 0;
  } else if (output < this->minimum_output_) {
    // shutdown boiler output below minimum value
    ESP_LOGI(TAG, "Calculated output below minimum value, forcing IDLE");
    output = 0;
  } else if (this->error_ <= -this->max_error_) {
    // shutdown boiler if ambiant temperature is too high
    ESP_LOGI(TAG, "Ambiant temperature exceeds max limit, forcing IDLE");
    output = 0;
  } else if (this->action == CLIMATE_ACTION_IDLE && this->error_ < 0) {
    // Don't restart boiler is ambiant temperature is above target
    ESP_LOGI(TAG, "Ambiant temperature above target, already on IDLE, forcing IDLE");
    output = 0;
  }
  if (this->heat_required_ && output < this->heat_required_output_) {
    // if heat required by switch, minimum output is heat_required_output_
    ESP_LOGI(TAG, "Forcing minimum output (heat required)");
    output = this->heat_required_output_;
  }

  ESP_LOGD(TAG, "Calculated output: %.1f%%", output * 100);

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
  } else if (this->mode == CLIMATE_MODE_OFF) {
    new_action = CLIMATE_ACTION_OFF;
    this->water_temp_ = NAN;
    this->reset_integral_term();
  } else {
    new_action = CLIMATE_ACTION_IDLE;
    this->water_temp_ = NAN;
  }

  ESP_LOGI(TAG, "Climate action was %s and is now %s", LOG_STR_ARG(climate_action_to_string(this->action)),
           LOG_STR_ARG(climate_action_to_string(new_action)));

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
}

void HeatingCurveClimate::calculate_integral_term_() {
  if (this->ki_ > 0) {
    // i(t) := K_i * \int_{0}^{t} e(t) dt
    float new_integral = this->error_ * this->dt_ * this->ki_;

    // Preventing integral wind up
    if (this->output_value_ <= this->minimum_output_ && new_integral < 0) {
      return;
    }
    if (this->output_value_ >= this->maximum_output_ && new_integral > 0) {
      return;
    }

    if (!in_deadband()) {
      this->integral_term_ += new_integral;
    }
  }
}

float HeatingCurveClimate::temperature_to_output(float temp) {
  float output = temp * this->output_calibration_factor_ + this->output_calibration_offset_;
  return this->rounded_ ? round(output) / 100.0 : output / 100.0;
}

float HeatingCurveClimate::get_heat_curve_temp() {
  this->delta_ = this->target_temperature - this->outdoor_temp_;

  float flow_temp;
  if (this->alt_curve_) {
    // ESP_LOGD(TAG, " Using alternate heating curve");
    float delta = -this->delta_;
    flow_temp = this->target_temperature + this->shift_ -
                this->slope_ * delta * (1.4347 + 0.021 * delta + 247.9 * 0.000001 * delta * delta);
  } else {
    flow_temp = this->delta_ * this->slope_ + this->target_temperature + this->shift_;
  }

  ESP_LOGD(TAG,
           "  Delta T: %.1fK\n"
           "  Heating curve temperature: %.1f°C",
           this->delta_, flow_temp);

  return flow_temp;
}

}  // namespace heat_curve
}  // namespace climate
}  // namespace esphome
