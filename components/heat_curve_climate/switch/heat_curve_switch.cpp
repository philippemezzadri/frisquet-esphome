#include "heat_curve_switch.h"

#include "esphome/core/log.h"

namespace esphome
{
  namespace heat_curve
  {

    static const char *const TAG = "heat_curve.switch";

    void HeatCurveClimateSwitch::dump_config()
    {
      LOG_SWITCH("", "Heat Curve Climate Switch", this);
    }

    void HeatCurveClimateSwitch::setup()
    {
      ESP_LOGCONFIG(TAG, "Setting up Heat Curve climate Switch '%s'...", this->name_.c_str());

      bool initial_state = Switch::get_initial_state_with_restore_mode();

      ESP_LOGCONFIG(TAG, "  Initial state: %s", initial_state ? "true" : "false");

      if (initial_state)
      {
        this->turn_on();
      }
      else
      {
        this->turn_off();
      }
    }
    void HeatCurveClimateSwitch::write_state(bool state)
    {
      if (state)
      {
        this->parent_->set_heat_required(true);
      }
      else
      {
        this->parent_->set_heat_required(false);
      }
      this->publish_state(state);
    }

  } // namespace heat_curve
} // namespace esphome