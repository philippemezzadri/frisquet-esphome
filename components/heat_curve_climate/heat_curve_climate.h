#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/automation.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/api/custom_api_device.h"

namespace esphome
{
    namespace heat_curve
    {
        // Climate standard visual configuration
        static const uint8_t CLIMATE_TEMP_MIN = 7;         // Celsius
        static const uint8_t CLIMATE_TEMP_MAX = 28;        // Celsius
        static const float CLIMATE_TEMPERATURE_STEP = 0.1; // K

        // If output < MINIMUM_OUTPUT, output = 0
        static const uint8_t MINIMUM_OUTPUT = 10;

        class HeatCurveClimate : public climate::Climate, public Component, public api::CustomAPIDevice
        {
        public:
            void setup() override;
            void dump_config() override;
            void on_send_new_heat_curve(float heat_factor, float offset, float kp);

            void add_temperature_computed_callback(std::function<void()> &&callback)
            {
                water_temp_computed_callback_.add(std::move(callback));
            }

            void set_sensor(sensor::Sensor *sensor) { current_sensor_ = sensor; }
            void set_outdoor_sensor(sensor::Sensor *sensor) { outoor_sensor_ = sensor; }
            void set_output(output::FloatOutput *output) { output_ = output; }
            void set_heat_factor(float heatfactor) { heat_factor_ = heatfactor; }
            void set_offset(float offset) { offset_ = offset; }
            void set_kp(float kp) { kp_ = kp; }
            void set_output_calibration_factor(float factor) { output_calibration_factor_ = factor; }
            void set_output_calibration_offset(float offset) { output_calibration_offset_ = offset; }

            float get_water_temp() { return water_temp_; }
            float get_result() { return result_; }
            float get_proportional_term() { return proportional_term_; }
            float get_error() { return error_; }
            float get_delta() { return delta_; }

        protected:
            void control(const climate::ClimateCall &call) override;
            climate::ClimateTraits traits() override;
            void write_output();

            // Parameters & inputs
            float heat_factor_ = 1.7;
            float offset_ = 20;
            float kp_ = 0;
            float output_calibration_factor_ = 1;
            float output_calibration_offset_ = 0;
            float outdoor_temp_ = NAN;

            // Results
            float water_temp_;
            float output_value_;
            float result_;
            float error_ = NAN;
            float proportional_term_ = 0;
            float delta_ = NAN;

            CallbackManager<void()> water_temp_computed_callback_;

            climate::ClimateMode active_mode_;
            climate::ClimateAction active_action_;
            sensor::Sensor *current_sensor_{nullptr};
            sensor::Sensor *outoor_sensor_{nullptr};
            output::FloatOutput *output_{nullptr};
        };
    } // namespace heat_curve
} // namespace esphome
