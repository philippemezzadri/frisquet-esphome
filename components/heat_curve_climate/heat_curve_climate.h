#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/automation.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/output/float_output.h"

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

        class HeatCurveClimate : public climate::Climate, public Component
        {
        public:
            void setup() override;
            void dump_config() override;

            void set_sensor(sensor::Sensor *sensor) { current_sensor_ = sensor; }
            void set_outdoor_sensor(sensor::Sensor *sensor) { outoor_sensor_ = sensor; }
            void set_output(output::FloatOutput *output) { output_ = output; }
            void set_heat_factor(float heatfactor) { heat_factor_ = heatfactor; }
            void set_offset(float offset) { offset_ = offset; }
            void set_kp(float kp) { kp_ = kp; }
            void set_output_calibration_factor(float factor) { output_calibration_factor_ = factor; }
            void set_output_calibration_offset(float offset) { output_calibration_offset_ = offset; }
            float get_setpoint() { return water_temp_; }
            void add_temperature_computed_callback(std::function<void()> &&callback)
            {
                water_temp_computed_callback_.add(std::move(callback));
            }

        protected:
            void control(const climate::ClimateCall &call) override;
            climate::ClimateTraits traits() override;
            void write_output();

            float outdoor_temp_;
            float water_temp_;
            float heat_factor_ = 1.7;
            float offset_ = 20;
            float kp_ = 0;
            float output_calibration_factor_ = 1;
            float output_calibration_offset_ = 0;
            CallbackManager<void()> water_temp_computed_callback_;

            climate::ClimateMode active_mode_;
            climate::ClimateAction active_action_;
            sensor::Sensor *current_sensor_{nullptr};
            sensor::Sensor *outoor_sensor_{nullptr};
            output::FloatOutput *output_{nullptr};
        };

        template <typename... Ts>
        class HeatCurveSetControlParametersAction : public Action<Ts...>
        {
        public:
            HeatCurveSetControlParametersAction(HeatCurveClimate *parent) : parent_(parent) {}

            void play(Ts... x)
            {
                auto heatfactor = this->ki_.value(x...);
                auto offset = this->kd_.value(x...);
                auto kp = this->kp_.value(x...);

                this->parent_->set_heat_factor(heatfactor);
                this->parent_->set_offset(offset);
                this->parent_->set_kp(kp);
            }

        protected:
            TEMPLATABLE_VALUE(float, heatfactor)
            TEMPLATABLE_VALUE(float, offset)
            TEMPLATABLE_VALUE(float, kp)

            HeatCurveClimate *parent_;
        };

    } // namespace heat_curve
} // namespace esphome
