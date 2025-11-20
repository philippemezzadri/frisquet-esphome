#pragma once

#include "esphome/components/climate/climate.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace climate {
namespace heat_curve {

static const float THRESHOLD_HIGH{0.15};
static const float THRESHOLD_LOW{-0.15};
static const float KP_MULTIPLIER{0.2};

class HeatingCurveClimate : public Climate, public Component {
 public:
  void update();

  void set_sensor(sensor::Sensor *sensor) { current_sensor_ = sensor; }
  void set_outdoor_sensor(sensor::Sensor *sensor) { outoor_sensor_ = sensor; }
  void set_heat_required(bool value);
  void set_rounded(bool rounded) { rounded_ = rounded; }
  void set_alt_curve(bool alt) { alt_curve_ = alt; }
  void set_output(output::FloatOutput *output) { output_ = output; }
  void set_slope(float slope) { slope_ = slope; }
  void set_shift(float shift) { shift_ = shift; }
  void set_kp(float kp) { kp_ = kp; }
  void set_ki(float ki) { ki_ = ki; }
  void set_max_error(float max) { max_error_ = max; }
  void set_min_delta(float min) { min_delta_ = min; }
  void set_minimum_output(float min) { minimum_output_ = min; }
  void set_maximum_output(float max) { maximum_output_ = max; }
  void set_heat_required_output(float heatreq_out) { heat_required_output_ = heatreq_out; }
  void set_output_calibration_factor(float factor) { output_calibration_factor_ = factor; }
  void set_output_calibration_offset(float offset) { output_calibration_offset_ = offset; }
  void reset_integral_term() { integral_term_ = 0; }

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

  void add_temperature_computed_callback(std::function<void()> &&callback) {
    internal_sensor_callback_.add(std::move(callback));
  }

  float get_output_value() const { return output_value_; }
  float get_water_temp() { return water_temp_; }
  float get_proportional_term() { return proportional_term_; }
  float get_integral_term() const { return integral_term_; }
  float get_error() { return error_; }
  float get_delta() { return delta_; }
  float get_slope() { return slope_; }
  float get_shift() { return shift_; }
  float get_kp() { return kp_; }
  float get_ki() { return ki_; }
  float output_to_temperature(float output);
  float temperature_to_output(float temp);
  float get_heat_curve_temp();

  void set_default_target_temperature(float default_target_temperature) {
    default_target_temperature_ = default_target_temperature;
  }

 protected:
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  void write_output_(float value);

  float calculate_relative_time_();
  bool in_deadband();
  void calculate_proportional_term_();
  void calculate_integral_term_();

  // Parameters & inputs
  float slope_{1.5};
  float dt_{0};
  float kp_{0};
  float ki_{0};
  float shift_{0};
  float outdoor_temp_{NAN};
  float output_calibration_factor_{1};
  float output_calibration_offset_{0};
  float minimum_output_{0};
  float maximum_output_{1};
  float heat_required_output_{0.1};
  float min_delta_{2.0};
  float max_error_{1.0};
  bool heat_required_{false};
  bool rounded_{false};
  bool alt_curve_{false};

  uint32_t last_time_{0};

  // Sensors
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *outoor_sensor_{nullptr};
  output::FloatOutput *output_{nullptr};

  CallbackManager<void()> internal_sensor_callback_;
  float default_target_temperature_;
  bool do_publish_ = false;

  // Results
  float output_value_{0};
  float water_temp_{20};
  float delta_{NAN};
  float error_{NAN};
  float integral_term_{0};
  float proportional_term_{0};
};

template<typename... Ts> class SetControlParametersAction : public Action<Ts...> {
 public:
  SetControlParametersAction(HeatingCurveClimate *parent) : parent_(parent) {}

  void play(const Ts &...x) {
    auto slope = this->slope_.value(x...);
    auto shift = this->shift_.value(x...);
    auto kp = this->kp_.value(x...);
    auto ki = this->ki_.value(x...);
    auto max_error = this->max_error_.value(x...);
    auto min_delta = this->min_delta_.value(x...);

    this->parent_->set_slope(slope);
    this->parent_->set_shift(shift);
    this->parent_->set_kp(kp);
    this->parent_->set_ki(ki);
    this->parent_->set_max_error(max_error);
    this->parent_->set_min_delta(min_delta);
    this->parent_->dump_config();
    this->parent_->update();
  }

 protected:
  TEMPLATABLE_VALUE(float, slope)
  TEMPLATABLE_VALUE(float, shift)
  TEMPLATABLE_VALUE(float, kp)
  TEMPLATABLE_VALUE(float, ki)
  TEMPLATABLE_VALUE(float, max_error)
  TEMPLATABLE_VALUE(float, min_delta)

  HeatingCurveClimate *parent_;
};

template<typename... Ts> class PIDResetIntegralTermAction : public Action<Ts...> {
 public:
  PIDResetIntegralTermAction(HeatingCurveClimate *parent) : parent_(parent) {}

  void play(const Ts &...x) { this->parent_->reset_integral_term(); }

 protected:
  HeatingCurveClimate *parent_;
};

}  // namespace heat_curve
}  // namespace climate
}  // namespace esphome
