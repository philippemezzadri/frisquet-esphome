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

const uint8_t CLIMATE_TEMP_MIN = 7;  // Celsius
const uint8_t CLIMATE_TEMP_MAX = 28; // Celsius
const float SLOPE = 1.7;
const float PIVOT_POINT = 23;

class HeatCurveClimate : public Climate, public Component
{
private:
    char const *TAG = "heat_curve.climate";
    float outdoor_temp;
    float water_temp;
    float slope_ = SLOPE;
    float pivot_ = PIVOT_POINT;

    ClimateMode active_mode_;
    sensor::Sensor *current_sensor_{nullptr};
    sensor::Sensor *outoor_sensor_{nullptr};
    sensor::Sensor *water_temp_sensor_{nullptr};
    output::FloatOutput *output_{nullptr};

public:
    void
    setup() override
    {
        if (this->current_sensor_)
        {
            this->current_sensor_->add_on_state_callback([this](float state)
                                                         {
                                                             this->current_temperature = state;
                                                             this->publish_state();
                                                         });
            this->current_temperature = this->current_sensor_->state;
        }
        else
            this->current_temperature = NAN;

        if (this->outoor_sensor_)
        {
            this->outoor_sensor_->add_on_state_callback([this](float state)
                                                        {
                                                            this->outdoor_temp = state;
                                                            this->water_temp = floor(((this->target_temperature - this->outdoor_temp) * this->slope_ + this->pivot_) + 0.5);
                                                            this->water_temp = clamp(this->water_temp, 0.0f, 100.0f);

                                                            if (this->mode == climate::CLIMATE_MODE_OFF)
                                                            {
                                                                ESP_LOGD(TAG, "Climate mode is OFF");
                                                                this->action = climate::CLIMATE_ACTION_OFF;
                                                                this->water_temp = 0;
                                                            }
                                                            else if (this->outdoor_temp > this->target_temperature - 3 || this->water_temp < 25)
                                                            {
                                                                ESP_LOGD(TAG, "Climate action is IDLE");
                                                                this->action = climate::CLIMATE_ACTION_IDLE;
                                                                this->water_temp = 0;
                                                            }
                                                            else
                                                                this->action = climate::CLIMATE_ACTION_HEATING;

                                                            this->publish_state();
                                                            ESP_LOGD(TAG, "Water temperature setpoint: %.1f°C", this->water_temp);

                                                            if (this->water_temp_sensor_)
                                                                this->water_temp_sensor_->publish_state(this->water_temp);

                                                            if (this->output_)
                                                                this->output_->set_level(this->water_temp / 100.0);
                                                        });
            this->outdoor_temp = this->outoor_sensor_->state;
        }
        else
            this->outdoor_temp = 15.0; // Default outdoor temp if none is available

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

    void set_sensor(sensor::Sensor *sensor) { this->current_sensor_ = sensor; }
    void set_outdoor_sensor(sensor::Sensor *sensor) { this->outoor_sensor_ = sensor; }
    void set_water_temp_sensor(sensor::Sensor *sensor) { this->water_temp_sensor_ = sensor; }
    void set_output(output::FloatOutput *output) { this->output_ = output; }
    void set_slope(float slope) { this->slope_ = slope; }
    void set_pivot(float pivot) { this->pivot_ = pivot; }

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
    }

    /// Return the traits of this controller
    climate::ClimateTraits traits() override
    {
        auto traits = climate::ClimateTraits();
        traits.set_supports_current_temperature(this->current_sensor_ != nullptr);
        traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT});
        traits.set_supports_two_point_target_temperature(false);
        traits.set_visual_min_temperature(CLIMATE_TEMP_MIN);
        traits.set_visual_max_temperature(CLIMATE_TEMP_MAX);
        traits.set_visual_temperature_step(0.1);
        return traits;
    }
};
