#pragma once

#include "esphome/core/component.h"
#include "../heat_curve_climate.h"

namespace esphome
{
    namespace heat_curve
    {

        enum HeatCurveClimateSensorType
        {
            PID_SENSOR_TYPE_RESULT,
            PID_SENSOR_TYPE_SETPOINT,
            PID_SENSOR_TYPE_WATERTEMP,
            PID_SENSOR_TYPE_ERROR,
            PID_SENSOR_TYPE_DELTA,
            PID_SENSOR_TYPE_PROPORTIONAL,
        };

        class HeatCurveClimateSensor : public sensor::Sensor, public Component
        {
        public:
            void setup() override;
            void set_parent(HeatCurveClimate *parent) { parent_ = parent; }
            void set_type(HeatCurveClimateSensorType type) { type_ = type; }
            void dump_config() override;

        protected:
            void update_from_parent_();
            HeatCurveClimate *parent_;
            HeatCurveClimateSensorType type_;
        };

    } // namespace pid
} // namespace esphome