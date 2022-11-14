/**
 * @file HeatCurveClimate.h
 * @author Philippe Mezzadri (philippe@mezzadri.fr)
 *
 * @version 0.2
 * @date 2022-01-19
 *
 * MIT License
 *
 * @copyright (c) 2022 Philippe Mezzadri
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "esphome.h"

using namespace esphome;

// Climate standard visual configuration
static const uint8_t CLIMATE_TEMP_MIN = 7;         // Celsius
static const uint8_t CLIMATE_TEMP_MAX = 28;        // Celsius
static const float CLIMATE_TEMPERATURE_STEP = 0.1; // K

// If output < MINIMUM_OUTPUT, output = 0
static const uint8_t MINIMUM_OUTPUT = 10;

class HeatCurveClimate : public Climate, public Component
{
protected:
    char const *TAG = "heat_curve.climate";
    float outdoor_temp_;
    float water_temp_;
    float output_conversion_factor_ = 1;
    float output_conversion_offset_ = 0;

    ClimateMode active_mode_;
    ClimateAction active_action_;
    sensor::Sensor *current_sensor_{nullptr};
    sensor::Sensor *outoor_sensor_{nullptr};
    sensor::Sensor *water_temp_sensor_{nullptr};
    output::FloatOutput *output_{nullptr};

public:
    void set_sensor(sensor::Sensor *sensor) { this->current_sensor_ = sensor; }
    void set_outdoor_sensor(sensor::Sensor *sensor) { this->outoor_sensor_ = sensor; }
    void set_water_temp_sensor(sensor::Sensor *sensor) { this->water_temp_sensor_ = sensor; }
    void set_output(output::FloatOutput *output) { this->output_ = output; }
    void set_output_conversion_factor(float factor) { this->output_conversion_factor_ = factor; }
    void set_output_conversion_offset(float offset) { this->output_conversion_offset_ = offset; }

    /// Return the traits of this controller
    climate::ClimateTraits traits() override
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

    void setup() override
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
                                                                 this->set_output();
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
                                                            this->set_output(); });
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
    }

    /// Override control to change settings of the climate device
    void control(const climate::ClimateCall &call) override
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

    void set_output()
    {
        float new_temp;
        float output;

        // New return water temperature according to heat curve
        new_temp = (this->target_temperature - this->outdoor_temp_) * global_heat_factor->value() + global_offset->value();

        // Proportional correction to accelerate conversion to setpoint
        if (!isnan(this->current_temperature) && !isnan(this->target_temperature))
            new_temp -= global_kp->value() * (this->current_temperature - this->target_temperature);

        ESP_LOGD(TAG, "Calculated temperature: %.1f°C", new_temp);

        // Boiler setpoint calculation according to water temperatur and conversion factors
        output = floor(new_temp * this->output_conversion_factor_ + this->output_conversion_offset_);
        output = clamp(output, 0.0f, 100.0f);

        // Recalculate actual return water temperature (knowing that the output is an integer)
        new_temp = (output - this->output_conversion_offset_) / this->output_conversion_factor_;

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

        if (this->water_temp_sensor_ && new_temp != this->water_temp_)
        {
            this->water_temp_ = new_temp;
            this->water_temp_sensor_->publish_state(this->water_temp_);
            ESP_LOGI(TAG, "New water temperature setpoint: %.1f°C", this->water_temp_);
        }

        ESP_LOGD(TAG, "Water temperature setpoint: %.1f°C", this->water_temp_);

        if (this->output_)
            this->output_->set_level(output / 100.0);
    }
};
