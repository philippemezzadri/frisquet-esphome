#include "frisquet_boiler.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace frisquet_boiler {
static const char *TAG = "frisquet.output";

void FrisquetBoiler::setup() {
  // Setup GPIO pin
  this->pin_->setup();
  this->digital_write(LOW);

  // Init cycle delay for first message
  this->delay_cycle_cmd_ = DELAY_CYCLE_CMD_INIT;
}

void FrisquetBoiler::set_mode(int mode) {
#ifdef USE_SWITCH
  this->mode_ = mode;
  if (mode == TEST_MODE) {
    this->msg_counter_ = 0;
    if (this->test_switch_ != nullptr) {
      this->pair_switch_->state = false;
      this->pair_switch_->publish_state(false);
    }

  } else if (mode == CONFIG_MODE) {
    if (this->test_switch_ != nullptr) {
      this->test_switch_->state = false;
      this->test_switch_->publish_state(false);
    }
  }

  if (this->msg_counter_ > 0) {
    this->log_last_message(this->comm_test_message_, LONG_MESSAGE_SIZE);
  }
#endif
}

void FrisquetBoiler::set_boiler_id(const char *str) {
  esphome::parse_hex(str, this->boiler_id_, 2);
  this->message_[4] = this->boiler_id_[0];
  this->message_[5] = this->boiler_id_[1];
  this->comm_test_message_[4] = this->boiler_id_[0];
  this->comm_test_message_[5] = this->boiler_id_[1];
  this->comm_setup_message_[12] = this->boiler_id_[0];
  this->comm_setup_message_[13] = this->boiler_id_[1];
}

void FrisquetBoiler::write_state(float state) {
  int new_demand = round(state * 100);

  // Cmd = 15 is known as not working
  // if (new_demand == 15)
  //   new_demand = 16;

  this->last_order_ = millis();

  if (new_demand != this->operating_setpoint_) {
    this->operating_setpoint_ = new_demand;
    ESP_LOGD(TAG, "New heating demand: %.3f", state);
    ESP_LOGD(TAG, "New boiler setpoint: (%i, %i)", this->operating_mode_, this->operating_setpoint_);
    this->send_message();
    this->last_cmd_ = this->last_order_;
    this->delay_cycle_cmd_ = DELAY_REPEAT_CMD;
    this->calculate_flow_temperature();
  }
}

void FrisquetBoiler::loop() {
  if (this->mode_ == CONTROL_MODE) {
    long now = millis();
    if ((now - this->last_cmd_ > this->delay_cycle_cmd_) &&
        ((now - this->last_order_ < DELAY_TIMEOUT_CMD_MQTT) || (DELAY_TIMEOUT_CMD_MQTT == 0))) {
      ESP_LOGD(TAG, "Repeating last command");
      this->send_message();
      this->last_cmd_ = now;
      this->delay_cycle_cmd_ = DELAY_CYCLE_CMD;
    }
  } else if (this->mode_ == TEST_MODE) {
    this->send_test_message();
  } else if (this->mode_ == CONFIG_MODE) {
    this->send_pairing_message();
  }
}

void FrisquetBoiler::dump_config() {
  ESP_LOGCONFIG(TAG, "Frisquet Boiler Output");
  ESP_LOGCONFIG(TAG, "  Boiler ID: 0x%.2x%.2x", this->boiler_id_[0], this->boiler_id_[1]);
  ESP_LOGCONFIG(TAG, "  Calibration factor: %.2f", this->output_calibration_factor_);
  ESP_LOGCONFIG(TAG, "  Calibration offset: %.2f", this->output_calibration_offset_);
  LOG_PIN("  Pin: ", this->pin_);
  LOG_FLOAT_OUTPUT(this);
}

void FrisquetBoiler::set_operating_mode(int mode) {
  // new operating mode : 0 = eco / 3 = confort / 4 = hors gel
  if ((mode == 0) or (mode == 3) or (mode == 4)) {
    ESP_LOGD(TAG, "New mode: %i", mode);
    this->operating_mode_ = mode;
    this->last_order_ = millis();
  } else {
    ESP_LOGW(TAG, "New mode not valid: %i", mode);
  }
}

void FrisquetBoiler::send_message() {
  ESP_LOGI(TAG, "Sending command to the boiler : (%i, %i)", this->operating_mode_, this->operating_setpoint_);

  // Emits a serie of 3 messages to the ERS (Eco Radio System) input of the boiler
  for (uint8_t msg = 0; msg < 3; msg++) {
    // /!\ I had to put previous_state_ at HIGH to get the message decoded properly.
    this->previous_state_ = HIGH;
    this->message_[9] = msg;
    this->message_[10] = (msg == 2) ? this->operating_mode_ : this->operating_mode_ + 0x80;
    this->message_[11] = this->operating_setpoint_;

    int checksum = 0;
    for (uint8_t i = 4; i <= MESSAGE_SIZE - 4; i++)
      checksum -= this->message_[i];

    this->message_[13] = (uint8_t) ((checksum) >> 8);    // highbyte
    this->message_[14] = (uint8_t) ((checksum) & 0xff);  // lowbyte

    for (uint8_t i = 1; i <= MESSAGE_SIZE; i++)
      this->serialize_byte(this->message_[i], i, MESSAGE_SIZE);

    this->digital_write(LOW);
    delay(DELAY_BETWEEN_MESSAGES);
  }

  // /!\ Final transition necessary to get the last message decoded properly;
  this->digital_write(HIGH);
  delayMicroseconds(2 * LONG_PULSE);
  this->digital_write(LOW);
  this->log_last_message(this->message_, MESSAGE_SIZE);
}

/**
 * Serialize a byte to the ERS input of the boiler
 *
 * @param byteValue Byte value to be serialized
 * @param byteIndex Order of the byte in the message, used for bit stuffing
 */
void FrisquetBoiler::serialize_byte(uint8_t byteValue, uint8_t byteIndex, uint8_t msgSize) {
  for (uint8_t n = 0; n < 8; n++) {
    int bitValue = ((byteValue >> n) & 0x1);  // bitread
    this->write_bit(bitValue);

    // bit stuffing only applicable to the data part of the message (bytes 4 to 16)
    // increment bitstuffing counter if bitValue == 1
    if (byteIndex >= 4 && byteIndex <= msgSize - 2 && bitValue == 1)
      this->bitstuff_counter_++;

    // reset bitstuffing counter
    if (bitValue == 0)
      this->bitstuff_counter_ = 0;

    if (this->bitstuff_counter_ >= 5) {
      // After 5 consecutive '1', insert a '0' bit (bitstuffing) and reset counter
      this->write_bit(0);
      this->bitstuff_counter_ = 0;
    }
  }
}

/**
 * Emits a signal to the ERS input corresponding to the given bitValue
 *
 * Signal level alternates after each bit
 * 0 : LOW LOW or HIGH HIGH(long pulse)
 * 1 : LOW HIGH or HIGH LOW(short pulse)
 *
 * @param bitValue Bit to be sent on pin
 */
void FrisquetBoiler::write_bit(bool bitValue) {
  this->previous_state_ = !this->previous_state_;
  this->digital_write(this->previous_state_);
  delayMicroseconds(LONG_PULSE);

  // if bit == 1, put transition
  if (bitValue) {
    this->previous_state_ = !this->previous_state_;
    this->digital_write(this->previous_state_);
  }

  delayMicroseconds(LONG_PULSE);
}

void FrisquetBoiler::log_last_message(uint8_t *msg, uint8_t length) {
  char const *formatString = "%02X";
  char *buffer = (char *) malloc(100 * sizeof(char));
  char *endofBuffer = buffer;
  int valueCount = length;
  int i;
  for (i = 0; i < valueCount; ++i) {
    endofBuffer += sprintf(endofBuffer, formatString, msg[i + 1]);
    if (i < valueCount - 1)
      endofBuffer += sprintf(endofBuffer, "%c", ' ');
  }

  ESP_LOGD(TAG, "Last message frames: %s", buffer);
  free(buffer);
}

void FrisquetBoiler::calculate_flow_temperature() {
  this->flow_temperature_ =
      this->operating_setpoint_ > 0
          ? (this->operating_setpoint_ - this->output_calibration_offset_) / this->output_calibration_factor_
          : NAN;

  this->internal_sensor_callback_.call();
}

void FrisquetBoiler::send_test_message() {
  ESP_LOGI(TAG, "Sending test command to the boiler");

  // Emits a serie of 2 test messages to the ERS (Eco Radio System) input of the boiler
  for (uint8_t msg = 0; msg < 3; msg++) {
    this->previous_state_ = HIGH;
    this->comm_test_message_[9] = 0xE0 + msg;
    this->comm_test_message_[10] = this->msg_counter_ & 0xff;

    int checksum = 0;
    for (uint8_t i = 4; i <= LONG_MESSAGE_SIZE - 4; i++)
      checksum -= this->comm_test_message_[i];

    this->comm_test_message_[18] = (uint8_t) ((checksum) >> 8);    // highbyte
    this->comm_test_message_[19] = (uint8_t) ((checksum) & 0xff);  // lowbyte

    for (uint8_t i = 1; i <= LONG_MESSAGE_SIZE; i++)
      this->serialize_byte(this->comm_test_message_[i], i, LONG_MESSAGE_SIZE);

    this->digital_write(LOW);
    delay(DELAY_BETWEEN_MESSAGES);
  }

  // /!\ Final transition necessary to get the last message decoded properly;
  this->digital_write(HIGH);
  delayMicroseconds(2 * LONG_PULSE);
  this->digital_write(LOW);
  this->msg_counter_++;
}

void FrisquetBoiler::send_pairing_message() {
  ESP_LOGI(TAG, "Sending configuration command to the boiler");

  // Emits a serie of 2 test messages to the ERS (Eco Radio System) input of the boiler
  for (uint8_t msg = 0; msg < 3; msg++) {
    this->previous_state_ = HIGH;
    this->comm_setup_message_[9] = 0xF0 + msg;

    int checksum = 0;
    for (uint8_t i = 4; i <= LONG_MESSAGE_SIZE - 4; i++)
      checksum -= this->comm_setup_message_[i];

    this->comm_setup_message_[18] = (uint8_t) ((checksum) >> 8);    // highbyte
    this->comm_setup_message_[19] = (uint8_t) ((checksum) & 0xff);  // lowbyte

    for (uint8_t i = 1; i <= LONG_MESSAGE_SIZE; i++)
      this->serialize_byte(this->comm_setup_message_[i], i, LONG_MESSAGE_SIZE);

    this->digital_write(LOW);
    delay(DELAY_BETWEEN_MESSAGES);
  }

  // /!\ Final transition necessary to get the last message decoded properly;
  this->digital_write(HIGH);
  delayMicroseconds(2 * LONG_PULSE);
  this->digital_write(LOW);
}

}  // namespace frisquet_boiler
}  // namespace esphome
