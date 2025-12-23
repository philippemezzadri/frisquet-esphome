#pragma once

#include "esphome/components/output/float_output.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"

#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif

// HIGH and LOW are defined in esp32-hal-gpio.h

#ifndef HIGH
#define HIGH 0x1
#endif

#ifndef LOW
#define LOW 0x0
#endif

namespace esphome {
namespace frisquet_boiler {

static const int DELAY_CYCLE_CMD{240000};         // delay between 2 commands (4min)
static const int DELAY_CYCLE_CMD_INIT{240000};    // delay for the 1st command after startup (4min)
static const int DELAY_REPEAT_CMD{20000};         // when a new command is issued, it is repeated after this delay (20s)
static const int DELAY_TIMEOUT_CMD_MQTT{900000};  // 15min Max delay without msg (against overheating) - 0 to deactivate
static const int DELAY_BETWEEN_MESSAGES{33};      // ms
static const int LONG_PULSE{825};                 // micro seconds

static const int CONTROL_MODE{0};
static const int TEST_MODE{1};
static const int CONFIG_MODE{2};

static const int MESSAGE_SIZE{16};
static const int LONG_MESSAGE_SIZE{21};

class FrisquetBoiler : public output::FloatOutput, public Component {
#ifdef USE_SWITCH
  SUB_SWITCH(test)
  SUB_SWITCH(pair)
#endif

 public:
  void setup() override;
  void loop() override;
  void write_state(float state) override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::IO; }

  void set_mode(int mode);
  void set_operating_setpoint(int setpoint) { this->operating_setpoint_ = setpoint; }
  void set_pin(GPIOPin *pin) { pin_ = pin; }
  void set_operating_mode(int mode);
  void set_boiler_id(const char *str);
  void set_output_calibration_factor(float factor) { output_calibration_factor_ = factor; }
  void set_output_calibration_offset(float offset) { output_calibration_offset_ = offset; }
  void calculate_flow_temperature();

  void add_sensor_callback(std::function<void()> &&callback) { internal_sensor_callback_.add(std::move(callback)); }

  float get_flow_temperature() { return flow_temperature_; }
  int get_setpoint() { return operating_setpoint_; }

 protected:
  void digital_write(bool value) { this->pin_->digital_write(value); }
  void send_message();
  void send_pairing_message();
  void send_test_message();
  void serialize_byte(uint8_t byteValue, uint8_t byteIndex, uint8_t msgSize);
  void write_bit(bool bitValue);
  void log_last_message(uint8_t *msg, uint8_t length);

  CallbackManager<void()> internal_sensor_callback_;

  GPIOPin *pin_;
  int mode_{CONTROL_MODE};
  int operating_mode_{3};
  int operating_setpoint_{0};
  int previous_state_{LOW};
  int bitstuff_counter_{0};
  int msg_counter_{0};
  int delay_cycle_cmd_;  //  This variable contains the delay for the next command to the boiler (if no order is
                         //  received)
  long last_cmd_{0};
  long last_order_{0};

  uint8_t message_[17] = {0x00, 0x00, 0x00, 0x7E, 0x03, 0xB9, 0x00, 0x20, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0xFD, 0x00, 0xFF, 0x00};

  uint8_t comm_test_message_[22] = {0x00, 0x00, 0x00, 0x7E, 0x03, 0xB9, 0x00, 0xFF, 0x00, 0xE0, 0x08,
                                    0x01, 0x00, 0xFF, 0xFF, 0xFF, 0x7D, 0xDF, 0x00, 0x00, 0xFF, 0x00};

  uint8_t comm_setup_message_[22] = {0x00, 0x00, 0x00, 0x7E, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xF0, 0xFF,
                                     0xFF, 0x03, 0xB9, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0xFF, 0x00};

  uint8_t boiler_id_[2];
  float output_calibration_factor_{1.9};
  float output_calibration_offset_{-41};
  float flow_temperature_{NAN};
};

template<typename... Ts> class SetModeAction : public Action<Ts...> {
 public:
  SetModeAction(FrisquetBoiler *output) : output_(output) {}

  TEMPLATABLE_VALUE(int, mode)

  void play(const Ts &...x) { this->output_->set_operating_mode(this->mode_.value(x...)); }

 protected:
  FrisquetBoiler *output_;
};

}  // namespace frisquet_boiler
}  // namespace esphome
