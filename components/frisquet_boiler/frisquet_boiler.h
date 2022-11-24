#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"

namespace esphome
{
    namespace frisquet_boiler
    {
        static const int DELAY_CYCLE_CMD = 240000;        // delay between 2 commands (4min)
        static const int DELAY_CYCLE_CMD_INIT = 240000;   // delay for the 1st command after startup (4min)
        static const int DELAY_REPEAT_CMD = 20000;        // when a new command is issued, it is repeated after this delay (20s)
        static const int DELAY_TIMEOUT_CMD_MQTT = 900000; // 15min Max delay without Mqtt msg ---PROTECTION OVERHEATING ---- (Same as remote) - 0 to deactivate
        static const int DELAY_BETWEEN_MESSAGES = 33;     // ms

        static const uint8_t ONBOARD_LED = 2;
        static const uint8_t ERS_PIN = 5;
        static const int LONG_PULSE = 825; // micro seconds

        class FrisquetBoiler : public output::FloatOutput, public Component
        {
        public:
            void setup() override;
            void write_state(float state) override;
            void loop() override;
            void dump_config() override;

            void blink();
            void set_operating_mode(int mode) { this->operating_mode_ = mode; }
            void set_operating_setpoint(int setpoint) { this->operating_setpoint_ = setpoint; }
            void on_send_operating_mode(int mode);
            void on_send_operating_setpoint(int setpoint);

        protected:
            int operating_mode_ = 3;
            int operating_setpoint_ = 0;
            int previous_state_ = LOW;
            int bitstuff_counter_ = 0;
            int delay_cycle_cmd_; //  This variable contains the delay for the next command to the boiler (if no order is received)
            long last_cmd_ = 0;
            long last_order_ = 0;
            uint8_t message_[17] = {0x00, 0x00, 0x00, 0x7E, 0x03, 0xB9, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0x00, 0xFF, 0x00};

            void send_message();
            void serialize_byte(uint8_t byteValue, uint8_t byteIndex);
            void write_bit(bool bitValue);
            void log_last_message();
        };

    } // namespace frisquet_boiler
} // namespace esphome
