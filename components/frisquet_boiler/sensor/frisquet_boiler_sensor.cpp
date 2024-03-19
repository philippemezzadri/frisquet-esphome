#include "frisquet_boiler_sensor.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace frisquet_boiler {

static const char *const TAG = "frisquet.output.sensor";

void FrisquetBoilerSensor::setup() {
  this->parent_->add_sensor_callback([this]() { this->update_from_parent_(); });
  this->update_from_parent_();
}
void FrisquetBoilerSensor::update_from_parent_() {
  float value;
  switch (this->type_) {
    case BOILER_SENSOR_TYPE_SETPOINT:
      value = this->parent_->get_setpoint();
      break;
    case BOILER_SENSOR_TYPE_FLOWTEMP:
      value = this->parent_->get_flow_temperature();
      break;
    default:
      value = NAN;
      break;
  }
  this->publish_state(value);
}
void FrisquetBoilerSensor::dump_config() { LOG_SENSOR("", "Frisquet Boiler Internal Sensor", this); }

}  // namespace frisquet_boiler
}  // namespace esphome
