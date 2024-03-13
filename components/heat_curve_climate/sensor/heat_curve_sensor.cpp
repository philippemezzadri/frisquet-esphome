#include "heat_curve_sensor.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace climate {
namespace heat_curve {

static const char *const TAG = "heating_curve.sensor";

void HeatCurveClimateSensor::setup() {
  this->parent_->add_temperature_computed_callback([this]() { this->update_from_parent_(); });
  this->update_from_parent_();
}
void HeatCurveClimateSensor::update_from_parent_() {
  float value;
  switch (this->type_) {
    case PID_SENSOR_TYPE_SETPOINT:
      value = this->parent_->get_output_value() * 100;
      break;
    case PID_SENSOR_TYPE_RESULT:
      value = this->parent_->get_output_value();
      break;
    case PID_SENSOR_TYPE_ERROR:
      value = this->parent_->get_error();
      break;
    case PID_SENSOR_TYPE_DELTA:
      value = this->parent_->get_delta();
      break;
    case PID_SENSOR_TYPE_WATERTEMP:
      value = this->parent_->get_water_temp();
      break;
    case PID_SENSOR_TYPE_PROPORTIONAL:
      value = this->parent_->get_proportional_term();
      break;
    case PID_SENSOR_TYPE_INTEGRAL:
      value = this->parent_->get_integral_term();
      break;
    case PID_SENSOR_TYPE_SLOPE:
      value = this->parent_->get_slope();
      break;
    case PID_SENSOR_TYPE_SHIFT:
      value = this->parent_->get_shift();
      break;
    case PID_SENSOR_TYPE_KP:
      value = this->parent_->get_kp();
      break;
    case PID_SENSOR_TYPE_KI:
      value = this->parent_->get_ki();
      break;
    default:
      value = NAN;
      break;
  }
  this->publish_state(value);
}
void HeatCurveClimateSensor::dump_config() { LOG_SENSOR("", "Heating Curve Climate Sensor", this); }

}  // namespace heat_curve
}  // namespace climate
}  // namespace esphome
