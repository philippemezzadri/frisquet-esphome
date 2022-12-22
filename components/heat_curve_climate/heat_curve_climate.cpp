#include "heat_curve_climate.h"

#include "esphome/core/log.h"

namespace esphome {
namespace heat_curve {
static const char *const TAG = "heat_curve.climate";

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

  // on state callback for outdorr temperature
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
    this->mode = climate::CLIMATE_MODE_HEAT;
    // initialize target temperature to some value so that it's not NAN
    this->target_temperature = roundf(this->current_temperature);
  }
}

/// Override control to change settings of the climate device
void HeatCurveClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value())
    this->mode = *call.get_mode();
  if (call.get_target_temperature().has_value())
    this->target_temperature = *call.get_target_temperature();

  // If switching to off mode, set output immediately
  if (this->mode == climate::CLIMATE_MODE_OFF)
    this->write_output_(0.0f);

  this->publish_state();
}

/// Return the traits of this controller
climate::ClimateTraits HeatCurveClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(this->current_sensor_ != nullptr);
  traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT});
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
  ESP_LOGCONFIG(TAG, "  Output Parameters:");
  ESP_LOGCONFIG(TAG, "    minimum_output_: %.2f", this->minimum_output_ / 100.0);
  ESP_LOGCONFIG(TAG, "    output_factor: %.2f", this->output_calibration_factor_);
  ESP_LOGCONFIG(TAG, "    output_offset: %.2f", this->output_calibration_offset_);
  this->dump_traits_(TAG);
}

void HeatCurveClimate::update() {
  float new_temp;
  float output;

  if (std::isnan(this->outdoor_temp_)) {
    ESP_LOGD(TAG, "Outdoor temperature not available, skipping calculation.");
    return;
  }

  // New return water temperature according to heat curve
  this->delta_ = this->target_temperature - this->outdoor_temp_;
  new_temp = this->delta_ * this->heat_factor_ + this->offset_;
  ESP_LOGD(TAG, "Delta T: %.1f", this->delta_);

  // Proportional correction to accelerate convergence to target
  if (!std::isnan(this->current_temperature) && !std::isnan(this->target_temperature)) {
    this->error_ = this->target_temperature - this->current_temperature;
    ESP_LOGD(TAG, "Error: %.1f", this->error_);

    this->proportional_term_ = this->kp_ * this->error_;
    new_temp += this->proportional_term_;
    ESP_LOGD(TAG, "Proportionnal term: %.1f", this->proportional_term_);
  }

  ESP_LOGD(TAG, "Calculated temperature: %.1f°C", new_temp);

  // Boiler setpoint calculation according to water temperature and calibration factors
  output = floor(new_temp * this->output_calibration_factor_ + this->output_calibration_offset_ + 0.5);
  output = clamp(output, 0.0f, 100.0f);

  // Recalculate actual water temperature (knowing that the output is an integer)
  new_temp = (output - this->output_calibration_offset_) / this->output_calibration_factor_;
  ESP_LOGD(TAG, "Calculated output: %.0f%%", output);
  ESP_LOGD(TAG, "Corrected temperature: %.1f°C", new_temp);
  this->water_temp_ = new_temp;

  // shutdown boiler if outdoor temperature is too high or output below minimum value
  if (this->outdoor_temp_ > this->target_temperature - 2 || output < this->minimum_output_) {
    output = 0;
  }

  if (this->mode == climate::CLIMATE_MODE_OFF) {
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
  climate::ClimateAction new_action;
  if (value > 0) {
    new_action = climate::CLIMATE_ACTION_HEATING;
    ESP_LOGD(TAG, "Climate action is HEATING");
  } else if (this->mode == climate::CLIMATE_MODE_OFF) {
    new_action = climate::CLIMATE_ACTION_OFF;
    this->water_temp_ = 20;
    ESP_LOGD(TAG, "Climate mode is OFF");
  } else {
    new_action = climate::CLIMATE_ACTION_IDLE;
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

}  // namespace heat_curve
}  // namespace esphome
