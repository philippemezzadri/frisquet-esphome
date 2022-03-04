/**
 * @file custom_component.h
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
using namespace output;

#define DELAY_CYCLE_CMD 240000        // delay between 2 commands (4min)
#define DELAY_CYCLE_CMD_INIT 240000   // delay for the 1st command after startup (4min)
#define DELAY_REPEAT_CMD 20000        // when a new command is issued, it is repeated after this delay (20s)
#define DELAY_TIMEOUT_CMD_MQTT 900000 // 15min Max delay without Mqtt msg ---PROTECTION OVERHEATING ---- (Same as remote) - 0 to deactivate
#define DELAY_BETWEEN_MESSAGES 33     // ms

static const uint8_t ONBOARD_LED = 2;
static const uint8_t ERS_PIN = 5;
static const int LONG_PULSE = 825; // micro seconds

class FrisquetBoilerFloatOutput : public Component, public CustomAPIDevice, public FloatOutput
{
private:
    char const *TAG = "frisquet.output";
    int previousState = LOW;
    int bitstuffCounter = 0;
    int delayCycleCmd; //  This variable contains the delay for the next command to the boiler (if no order is received)
    long lastCmd = 0;
    long lastOrder = 0;
    uint8_t message[17] = {0x00, 0x00, 0x00, 0x7E, 0x03, 0xB9, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0x00, 0xFF, 0x00};

public:
    int operating_setpoint = 0;
    int operating_mode = 3;

    void setup() override
    {
        /**
         * @brief This will be called once to set up the component
         *        Think of it as the setup() call in Arduino
         */

        // Init ESP32 pins modes
        pinMode(ERS_PIN, OUTPUT);
        digitalWrite(ERS_PIN, LOW);
        pinMode(ONBOARD_LED, OUTPUT);

        // Init cycle delay for first message
        delayCycleCmd = DELAY_CYCLE_CMD_INIT;

        // Set boiler id
        message[4] = 0x03;
        message[5] = 0xB9;

        // Register services
        register_service(&FrisquetBoilerFloatOutput::on_send_operating_mode, "send_operating_mode", {"mode"});
        register_service(&FrisquetBoilerFloatOutput::on_send_operating_setpoint, "send_operating_setpoint", {"setpoint"});
    }

    void write_state(float state) override
    {
        /**
         * @brief This will be called when the output state is updated
         *        The PI heating demand received from the controller is converted in water
         *        temperature setpoint.
         *
         * @param state PI regulation heating demand in [0, 1] range
         */

        int new_demand;
        new_demand = state * 100;

        if (new_demand == 15)
            new_demand = 14;

        lastOrder = millis();

        if (new_demand != operating_setpoint)
        {
            blink();
            ESP_LOGD(TAG, "New Heating demand: %.3f", state);

            operating_setpoint = new_demand;
            ESP_LOGD(TAG, "New boiler setpoint: (%i, %i)", operating_mode, operating_setpoint);
            send_message();
            lastCmd = lastOrder;
            delayCycleCmd = DELAY_REPEAT_CMD;
        }
    }

    void loop() override
    {
        /**
         * @brief This will be called very often after setup time.
         *        Think of it as the loop() call in Arduino
         */
        long now = millis();
        if ((now - lastCmd > delayCycleCmd) && ((now - lastOrder < DELAY_TIMEOUT_CMD_MQTT) || (DELAY_TIMEOUT_CMD_MQTT == 0)))
        {
            ESP_LOGD(TAG, "Sending messages");
            send_message();
            lastCmd = now;
            delayCycleCmd = DELAY_CYCLE_CMD;
        }
    }

    void on_send_operating_mode(int mode)
    {
        /**
         * @brief Service callable form Home Assistant frontend
         * @param mode new operating mode : 0 = eco / 3 = confort / 4 = hors gel
         */

        blink();
        if ((mode == 0) or (mode == 3) or (mode == 4))
        {
            ESP_LOGD(TAG, "New mode: %i", mode);
            operating_mode = mode;
            lastOrder = millis();
        }
        else
        {
            ESP_LOGW(TAG, "New mode not valid: %i", mode);
        }
    }

    void on_send_operating_setpoint(int setpoint)
    {
        /**
         * @brief Service callable form Home Assistant frontend
         * @param setpoint new water temperature setpoint
         *   0 : boiler stopped
         *   10 : water pump started
         *   20 - 100 : water temperature setpoint
         */

        blink();
        if ((setpoint >= 0) and (setpoint <= 100))
        {
            ESP_LOGD(TAG, "New setpoint: %i", setpoint);
            operating_setpoint = setpoint;
            send_message();
            lastCmd = millis();
            lastOrder = lastCmd;
            delayCycleCmd = DELAY_REPEAT_CMD;
        }
        else
        {
            ESP_LOGW(TAG, "New setpoint not valid: %i", setpoint);
        }
    }

    void blink()
    {
        /**
         * @brief makes the onboard LED blink once
         */

        digitalWrite(ONBOARD_LED, HIGH);
        delay(200);
        digitalWrite(ONBOARD_LED, LOW);
    }

private:
    /**
     * Below are all member functions related to the Frisquet Boiler communication protocol.
     */

    void send_message()
    {
        /**
         * @brief Emits a serie of 3 messages to the ERS input of the boiler
         */

        ESP_LOGI(TAG, "Sending command to boiler : (%i, %i)", operating_mode, operating_setpoint);
        blink();

        for (uint8_t msg = 0; msg < 3; msg++)
        {
            // /!\ I had to put previousState at HIGH to get the message decoded properly.
            previousState = HIGH;
            message[9] = msg;
            message[10] = (msg == 2) ? operating_mode : operating_mode + 0x80;
            message[11] = operating_setpoint;

            int checksum = 0;
            for (uint8_t i = 4; i <= 12; i++)
                checksum -= message[i];

            message[13] = highByte(checksum);
            message[14] = lowByte(checksum);

            for (uint8_t i = 1; i < 17; i++)
                serialize_byte(message[i], i);

            digitalWrite(ERS_PIN, LOW);
            delay(DELAY_BETWEEN_MESSAGES);
        }

        // /!\ Final transition necessary to get the last message decoded properly;
        digitalWrite(ERS_PIN, HIGH);
        delayMicroseconds(2 * LONG_PULSE);
        digitalWrite(ERS_PIN, LOW);
        log_last_message();
    }

    void serialize_byte(uint8_t byteValue, uint8_t byteIndex)
    {
        /**
         * @brief Serialize a byte to the ERS input of the boiler
         * @param byteValue byte value to be serialized
         * @param byteIndex order of the byte in the message, used for bit stuffing
         */

        for (uint8_t n = 0; n < 8; n++)
        {
            int bitValue = bitRead(byteValue, n);
            write_bit(bitValue);

            // bit stuffing only applicable to the data part of the message (bytes 4 to 16)
            // increment bitstuffing counter if bitValue == 1
            if (byteIndex >= 4 && byteIndex <= 14 && bitValue == 1)
                bitstuffCounter++;

            // reset bitstuffing counter
            if (bitValue == 0)
                bitstuffCounter = 0;

            if (bitstuffCounter >= 5)
            {
                // After 5 consecutive '1', insert a '0' bit (bitstuffing) and reset counter
                write_bit(0);
                bitstuffCounter = 0;
            }
        }
    }

    void write_bit(bool bitValue)
    {
        /**
         * @brief Emits a signal to the ERS input corresponding to the given bitValue
         *        Signal level alternates after each bit
         *        0 : LOW LOW or HIGH HIGH(long pulse)
         *        1 : LOW HIGH or HIGH LOW(short pulse)
         * @param bitValue bit to be sent on ERS_PIN
         */

        previousState = !previousState;
        digitalWrite(ERS_PIN, previousState);
        delayMicroseconds(LONG_PULSE);

        // if bit == 1, put transition
        if (bitValue)
        {
            previousState = !previousState;
            digitalWrite(ERS_PIN, previousState);
        }

        delayMicroseconds(LONG_PULSE);
    }

    void log_last_message()
    {
        /**
         * @brief Logs a copy of the last message
         */

        char const *formatString = "%02X";
        char *buffer = (char *)malloc(100 * sizeof(char));
        char *endofBuffer = buffer;
        int valueCount = 16;
        int i;
        for (i = 0; i < valueCount; ++i)
        {
            endofBuffer += sprintf(endofBuffer, formatString, message[i + 1]);
            if (i < valueCount - 1)
                endofBuffer += sprintf(endofBuffer, "%c", ' ');
        }

        ESP_LOGD(TAG, "last message: %s", buffer);
    }
};
