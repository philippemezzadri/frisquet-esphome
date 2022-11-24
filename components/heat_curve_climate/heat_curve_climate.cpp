#include "heat_curve_climate.h"
#include "esphome/core/log.h"

namespace esphome
{
    namespace heat_curve
    {
        static const char *const TAG = "heat_curve.climate";

        void HeatCurveClimate::setup()
        {
            // on state callback for current temperature
            if (this->current_sensor_)
            {
                this->current_sensor_->add_on_state_callback([this](float state)
                                                             {
                                                             if (state != this->current_temperature)
                                                             {
                                                                 this->current_temperature = state;
                                                                 this->publish_state();
                                                                 this->write_output();
                                                             } });
                this->current_temperature = this->current_sensor_->state;
            }
            else
                this->current_temperature = NAN;

            // on state callback for outdorr temperature
            if (this->outoor_sensor_)
            {
                this->outoor_sensor_->add_on_state_callback([this](float state)
                                                            {
                                                            this->outdoor_temp_ = state;
                                                            this->write_output(); });
                this->outdoor_temp_ = this->outoor_sensor_->state;
            }
            else
                this->outdoor_temp_ = 15.0; // Default outdoor temp if none is available

            // restore set points
            auto restore = this->restore_state_();
            if (restore.has_value())
            {
                restore->apply(this);
            }
            else
            {
                // restore from defaults
                this->mode = climate::CLIMATE_MODE_HEAT;
                // initialize target temperature to some value so that it's not NAN
                this->target_temperature = roundf(this->current_temperature);
            }
            this->active_mode_ = this->mode;

            // Register services
            register_service(&HeatCurveClimate::on_send_new_heat_curve, "send_new_heat_curve", {"heat_factor", "offset", "kp"});
        }

        /// Override control to change settings of the climate device
        void HeatCurveClimate::control(const climate::ClimateCall &call)
        {
            if (call.get_mode().has_value())
                this->mode = *call.get_mode();
            if (call.get_target_temperature().has_value())
                this->target_temperature = *call.get_target_temperature();

            if (this->mode == climate::CLIMATE_MODE_OFF)
            {
                this->action = climate::CLIMATE_ACTION_OFF;
                if (this->output_)
                    this->output_->set_level(0);
            }

            this->publish_state();
            this->active_mode_ = this->mode;
            this->active_action_ = this->action;
        }

        /// Return the traits of this controller
        climate::ClimateTraits HeatCurveClimate::traits()
        {
            auto traits = climate::ClimateTraits();
            traits.set_supports_current_temperature(this->current_sensor_ != nullptr);
            traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT});
            traits.set_supports_two_point_target_temperature(false);
            traits.set_supports_action(true);
            traits.set_visual_min_temperature(CLIMATE_TEMP_MIN);
            traits.set_visual_max_temperature(CLIMATE_TEMP_MAX);
            traits.set_visual_temperature_step(CLIMATE_TEMPERATURE_STEP);
            return traits;
        }

        void HeatCurveClimate::dump_config()
        {
            LOG_CLIMATE("", "Heat Factor Climate", this);
            ESP_LOGCONFIG(TAG, "  Control Parameters:");
            ESP_LOGCONFIG(TAG, "    heat_factor: %.2f, offset: %.2f, kp: %.2f", this->heat_factor_, this->offset_, this->kp_);
            ESP_LOGCONFIG(TAG, "    output_factor:%.2f, output_offset:%.2f", this->output_calibration_factor_, this->output_calibration_offset_);
        }

        void HeatCurveClimate::on_send_new_heat_curve(float heat_factor, float offset, float kp)
        {
            this->set_heat_factor(heat_factor);
            this->set_offset(offset);
            this->set_kp(kp);
            this->dump_config();
        }

        void HeatCurveClimate::write_output()
        {
            float new_temp;
            float output;

            // New return water temperature according to heat curve
            new_temp = (this->target_temperature - this->outdoor_temp_) * this->heat_factor_ + this->offset_;

            // Proportional correction to accelerate convergence to setpoint
            if (!std::isnan(this->current_temperature) && !std::isnan(this->target_temperature))
                new_temp -= this->kp_ * (this->current_temperature - this->target_temperature);

            ESP_LOGD(TAG, "Calculated temperature: %.1f°C", new_temp);

            // Boiler setpoint calculation according to water temperature and calibration factors
            output = floor(new_temp * this->output_calibration_factor_ + this->output_calibration_offset_ + 0.5);
            output = clamp(output, 0.0f, 100.0f);

            // Recalculate actual return water temperature (knowing that the output is an integer)
            new_temp = (output - this->output_calibration_offset_) / this->output_calibration_factor_;

            ESP_LOGD(TAG, "Calculated output: %.0f", output);
            ESP_LOGD(TAG, "Corrected temperature: %.1f°C", new_temp);

            // if CLIMATE_MODE_OFF, shutdown everything, output = 0
            if (this->mode == climate::CLIMATE_MODE_OFF)
            {
                ESP_LOGD(TAG, "Climate mode is OFF");
                this->action = climate::CLIMATE_ACTION_OFF;
                new_temp = 20;
                output = 0;
            }
            // shutdown boiler if outdoor temperature is too high or output below minimum value
            else if (this->outdoor_temp_ > this->target_temperature - 2 || output < MINIMUM_OUTPUT)
            {
                ESP_LOGD(TAG, "Climate action is IDLE");
                this->action = climate::CLIMATE_ACTION_IDLE;
                new_temp = 20;
                output = 0;
            }
            else
            {
                ESP_LOGD(TAG, "Climate action is HEATING");
                this->action = climate::CLIMATE_ACTION_HEATING;
            }

            // publich state only if there is a change to minimize flash memory writes
            if (this->active_action_ != this->action)
            {
                this->publish_state();
                this->active_action_ = this->action;
            }

            if (new_temp != this->water_temp_)
            {
                this->output_value_ = output;
                this->water_temp_ = new_temp;
                ESP_LOGI(TAG, "New water temperature setpoint: %.1f°C", this->water_temp_);
                this->water_temp_computed_callback_.call();
            }

            ESP_LOGD(TAG, "Water temperature setpoint: %.1f°C", this->water_temp_);

            if (this->output_)
            {
                this->output_->set_level(output / 100.0);
            }
        }
    }
}
