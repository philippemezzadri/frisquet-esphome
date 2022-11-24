#include "heat_curve_sensor.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome
{
    namespace heat_curve
    {

        static const char *const TAG = "heat_curve.sensor";

        void HeatCurveClimateSensor::setup()
        {
            this->parent_->add_temperature_computed_callback([this]()
                                                             { this->update_from_parent_(); });
            this->update_from_parent_();
        }
        void HeatCurveClimateSensor::update_from_parent_()
        {
            float value;
            switch (this->type_)
            {
            case PID_SENSOR_TYPE_SETPOINT:
                value = this->parent_->get_output_value();
                break;
            case PID_SENSOR_TYPE_WATERTEMP:
                value = this->parent_->get_water_setpoint();
                break;
            default:
                value = NAN;
                break;
            }
            this->publish_state(value);
        }
        void HeatCurveClimateSensor::dump_config() { LOG_SENSOR("", "Heat Curve Climate Sensor", this); }

    } // namespace pid
} // namespace esphome