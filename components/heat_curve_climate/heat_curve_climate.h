#pragma once

#include "esphome/components/climate/climate.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace heat_curve {

class HeatCurveClimate : public climate::Climate, public Component {
 public:
  void update();

  void set_sensor(sensor::Sensor *sensor) { current_sensor_ = sensor; }
  void set_outdoor_sensor(sensor::Sensor *sensor) { outoor_sensor_ = sensor; }
  void set_output(output::FloatOutput *output) { output_ = output; }
  void set_heat_factor(float heatfactor) { heat_factor_ = heatfactor; }
  void set_offset(float offset) { offset_ = offset; }
  void set_kp(float kp) { kp_ = kp; }
  void set_minimum_output(float min) { minimum_output_ = 100 * min; }
  void set_output_calibration_factor(float factor) { output_calibration_factor_ = factor; }
  void set_output_calibration_offset(float offset) { output_calibration_offset_ = offset; }

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

  void add_temperature_computed_callback(std::function<void()> &&callback) {
    water_temp_computed_callback_.add(std::move(callback));
  }

  float get_output_value() const { return output_value_; }
  float get_water_temp() { return water_temp_; }
  float get_proportional_term() { return proportional_term_; }
  float get_error() { return error_; }
  float get_delta() { return delta_; }

 protected:
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  void write_output_(float value);

  // Parameters & inputs
  float heat_factor_ = 1.7;
  float kp_ = 0;
  float offset_ = 20;
  float outdoor_temp_ = NAN;
  float output_calibration_factor_ = 1;
  float output_calibration_offset_ = 0;
  float minimum_output_ = 10;

  // Sensors
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *outoor_sensor_{nullptr};
  output::FloatOutput *output_{nullptr};

  CallbackManager<void()> water_temp_computed_callback_;
  bool do_publish_ = false;

  // Results
  float output_value_;
  float water_temp_;
  float delta_ = NAN;
  float error_ = NAN;
  float proportional_term_ = 0;
};

template <typename... Ts>
class SetControlParametersAction : public Action<Ts...> {
 public:
  SetControlParametersAction(HeatCurveClimate *parent) : parent_(parent) {}

  void play(Ts... x) {
    auto heat_factor = this->heat_factor_.value(x...);
    auto offset = this->offset_.value(x...);
    auto kp = this->kp_.value(x...);

    this->parent_->set_heat_factor(heat_factor);
    this->parent_->set_offset(offset);
    this->parent_->set_kp(kp);
    this->parent_->dump_config();
    this->parent_->update();
  }

 protected:
  TEMPLATABLE_VALUE(float, heat_factor)
  TEMPLATABLE_VALUE(float, offset)
  TEMPLATABLE_VALUE(float, kp)

  HeatCurveClimate *parent_;
};

}  // namespace heat_curve
}  // namespace esphome
