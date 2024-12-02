/*
Copyright <2018> <Ethan Johnston>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Arduino.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/ringbuf.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

#include "MilesTag.h"
////////////////////////////////////
#define DEBUG true
////////////////////////////////////

// Clock divisor (base clock is 80MHz)
#define CLK_DIV 80
#define TICK_10_US (80000000 / CLK_DIV / 100000)  // = 10
#define TIMEOUT_US 5                              // 50          //RMT receiver timeout value(us)

#define ROUND_TO 1         // 50          //rounding value for microseconds timings
#define MARK_EXCESS 0      // 100         //tweeked to get the right timing
#define SPACE_EXCESS 0     // 50          //tweeked to get the right timing
#define TIMEOUT_US 5       // 50          //RMT receiver timeout value(us)
#define MIN_CODE_LENGTH 5  // Minimum data pulses received for a valid packet

#define IR_PIN GPIO_NUM_21
#define REC_PIN GPIO_NUM_12

#define DEBUG_SCALE 1
#define CDEBUG 1

// #define SHOT_HEADER_US 3200  // Header when send a shot
// #define CMD_HEADER_US 4000   // Header when send a command
// #define CMD_EXTRA_US 5600    // Marker indecating extra data
// #define END_US 6400          // End Marker
// #define SPACE_US 800
// #define ONE_US 1600
// #define ZERO_US 800
// #define OFFSET 150

#define SHOT_HEADER_US 3000  // Header when send a shot
#define CMD_HEADER_US 4000   // Header when send a command
#define CMD_EXTRA_US 5000    // Marker indicating extra data
#define END_US 6000          // End Marker
#define SPACE_US 1000
#define ONE_US 2000
#define ZERO_US 1000
#define OFFSET 100

#define SIGNAL_ID_HIT 0x01
#define SIGNAL_ID_GRENADE 0x02
#define SIGNAL_ID_AMMO 0x03
#define SIGNAL_ID_MEDKIT 0x04

#define SIGNAL_ID_BASE 0x10
#define SIGNAL_ID_SPAWNPOINT 0x11
#define SIGNAL_ID_CAPTUREPOINT 0x12

enum IRSignalType { HIT, GRENADE, AMMUNITION, MEDKIT, BASE, SPAWNPOINT, CAPTUREPOINT };

void printBinary(uint16_t number, byte numberOfDigits) {
    uint16_t _number = number;
    byte _numberOfDigits = numberOfDigits;
    // #ifdef DEBUG
    int _mask = 0;
    for (byte _n = 1; _n <= _numberOfDigits; _n++) {
        _mask = (_mask << 1) | 0x0001;
    }
    _number = _number & _mask;  // truncate v to specified number of places

    while (_numberOfDigits) {
        if (_number & (0x0001 << (_numberOfDigits - 1))) {
            Serial.print(F("1"));
        } else {
            Serial.print(F("0"));
        }

        --_numberOfDigits;

        if (((_numberOfDigits % 4) == 0) && (_numberOfDigits != 0)) {
            Serial.print(F("_"));
        }
    }
    // #endif
    Serial.println("");
}

MilesTagTX::MilesTagTX() {
    if (DEBUG) Serial.print("ESP32_IR::Constructing");
}

MilesTagTX::MilesTagTX(int _txPin, int _channel) {
    if (DEBUG) Serial.print("ESP32_IR::Constructing");
    SetTx(_txPin, _channel);
}

bool MilesTagTX::SetTx(int _txPin, int _channel) {
    bool _status = true;

    if (_txPin >= GPIO_NUM_0 && _txPin < GPIO_NUM_MAX)
        txGpioNum = _txPin;
    else
        _status = false;

    if (_channel >= RMT_CHANNEL_0 && _channel < RMT_CHANNEL_MAX)
        txRmtPort = _channel;
    else
        _status = false;

    if (_status == false && DEBUG) Serial.println("ESP32_IR::Tx Pin init failed");
    return _status;
}

// transmit code
void MilesTagTX::txConfig() {
    rmt_config_t config;
    config.channel = (rmt_channel_t)txRmtPort;
    config.gpio_num = (gpio_num_t)txGpioNum;
    config.mem_block_num = 1;  // how many memory blocks 64 x N (0-7)
    config.clk_div = CLK_DIV;
    config.tx_config.loop_en = false;
    config.tx_config.carrier_duty_percent = 50;
    config.tx_config.carrier_freq_hz =56000;
    config.tx_config.carrier_level = (rmt_carrier_level_t)1;
    config.tx_config.carrier_en = 1;
    config.tx_config.idle_level = (rmt_idle_level_t)0;
    config.tx_config.idle_output_en = true;
    config.rmt_mode = (rmt_mode_t)0;  // RMT_MODE_TX;
    rmt_config(&config);
    rmt_driver_install(config.channel, 0, 0);  // 19     /*!< RMT interrupt number, select from soc.h */
}

void MilesTagTX::fireShot(u_int32_t playerId, u_int32_t dmg) { sendCommand(true, quantitytoBin(dmg), playerId); }

void MilesTagTX::sendCommand(bool shotCommand, uint8_t command, uint64_t data) {
    u_int32_t encodedData = 0;
    Serial.println("sendCommand");
    if (shotCommand) {
        Serial.println("short command");
        // Use 12 bits for short commands
        encodedData |= (command & 0xF) << 6;  // Encode command (bits 10-7)    - 4 bits
        encodedData |= (data & 0x3F);         // Encode player ID (bits 6-1)     - 6 bits
    } else {
        Serial.println("long command");
        // Use 22 bits for long commands
        encodedData |= (command & 0xF) << 16;  // Encode command (bits 20-17)  - 4 bits
        encodedData |= (data & 0xFFFF);        // Encode data (bits 16-1)      - 16 bits
    }

    // Add parity bit (bit 0)
    // Serial.println("encoded Data: " + String(encodedData, BIN));
    // encodedData = add_parity(encodedData);
    if (DEBUG) {
        Serial.println("encoded Data: " + String(encodedData, BIN));
    }

    // Send the data
    irTransmit(shotCommand, false, encodedData, shotCommand ? 10 : 20);
}

void MilesTagTX::irTransmit(bool shotCommand, bool extraData, u_int32_t data, int nbits) {
    Serial.println("irTransmit: Data: " + String(data, BIN) + " Bits " + String(nbits));
    // Header
    rmt_item32_t irDataArray[nbits + 6];  // start(1) + data(nbits) + checksum(4) + end(1)
    if (shotCommand) {
        irDataArray[0].duration0 = SHOT_HEADER_US * DEBUG_SCALE;
        irDataArray[0].level0 = 1;
        irDataArray[0].duration1 = SPACE_US * DEBUG_SCALE;
        irDataArray[0].level1 = 0;
    } else {
        irDataArray[0].duration0 = CMD_HEADER_US * DEBUG_SCALE;
        irDataArray[0].level0 = 1;
        irDataArray[0].duration1 = SPACE_US * DEBUG_SCALE;
        irDataArray[0].level1 = 0;
    }

    // Data
    for (int i = 1; i <= nbits; i++) {
        u_int32_t mask = 1ULL << (nbits - i);
        if (data & mask) {
            irDataArray[i].duration0 = ONE_US * DEBUG_SCALE;
            irDataArray[i].level0 = 1;
            irDataArray[i].duration1 = SPACE_US * DEBUG_SCALE;
            irDataArray[i].level1 = 0;
        } else {
            irDataArray[i].duration0 = ZERO_US * DEBUG_SCALE;
            irDataArray[i].level0 = 1;
            irDataArray[i].duration1 = SPACE_US * DEBUG_SCALE;
            irDataArray[i].level1 = 0;
        }
    }
    // Serial.println("data added" + String(nbits));
    u_int32_t checksum = calculateChecksum(data, nbits);
    for (int i = 0; i < 4; i++) {
        nbits++;
        if (checksum & (1UL << (3 - i))) {
            irDataArray[nbits].duration0 = ONE_US * DEBUG_SCALE;
            irDataArray[nbits].level0 = 1;
            irDataArray[nbits].duration1 = SPACE_US * DEBUG_SCALE;
            irDataArray[nbits].level1 = 0;
        } else {
            irDataArray[nbits].duration0 = ZERO_US * DEBUG_SCALE;
            irDataArray[nbits].level0 = 1;
            irDataArray[nbits].duration1 = SPACE_US * DEBUG_SCALE;
            irDataArray[nbits].level1 = 0;
        }
    }
    // Serial.println("Checksum added" + String(nbits));

    nbits++;
    if (extraData) {
        irDataArray[nbits].duration0 = CMD_EXTRA_US * DEBUG_SCALE;
        irDataArray[nbits].level0 = 1;
        irDataArray[nbits].duration1 = SPACE_US * DEBUG_SCALE;
        irDataArray[nbits].level1 = 0;
    } else {
        irDataArray[nbits].duration0 = END_US * DEBUG_SCALE;
        irDataArray[nbits].level0 = 1;
        irDataArray[nbits].duration1 = SPACE_US * DEBUG_SCALE;
        irDataArray[nbits].level1 = 0;
    }

    // Serial.println("Checksum added" + String(nbits));
    nbits++;
    if (DEBUG) {
        Serial.println("encoded Data: " + String(data, BIN) + " - " + String(checksum, BIN) + " - " + String(nbits) + " bits");
    }
    sendIR(irDataArray, nbits, true);
    Serial.println("Transmit Complete");
}

/**************************************************************************************************************************************************/

void MilesTagTX::sendIR(rmt_item32_t data[], int IRlength, bool waitTilDone) {
    rmt_config_t config;
    config.channel = (rmt_channel_t)txRmtPort;
    rmt_write_items(config.channel, data, IRlength, waitTilDone);  // false means non-blocking
    // Wait until sending is done.
    if (waitTilDone) {
        rmt_wait_tx_done(config.channel, 1);
    }
}

/**************************************************************************************************************************************************/

u_int32_t MilesTagTX::quantitytoBin(u_int32_t dmg) {
    if (dmg >= 100) {
        dmg = 15;
    } else if (dmg >= 75) {
        dmg = 14;
    } else if (dmg >= 50) {
        dmg = 13;
    } else if (dmg >= 40) {
        dmg = 12;
    } else if (dmg >= 35) {
        dmg = 11;
    } else if (dmg >= 30) {
        dmg = 10;
    } else if (dmg >= 25) {
        dmg = 9;
    } else if (dmg >= 20) {
        dmg = 8;
    } else if (dmg >= 17) {
        dmg = 7;
    } else if (dmg >= 15) {
        dmg = 6;
    } else if (dmg >= 10) {
        dmg = 5;
    } else if (dmg >= 7) {
        dmg = 4;
    } else if (dmg >= 5) {
        dmg = 3;
    } else if (dmg >= 4) {
        dmg = 2;
    } else if (dmg >= 2) {
        dmg = 1;
    } else if (dmg >= 1) {
        dmg = 0;
    }
    return dmg;
}

u_int32_t MilesTagTX::has_even_parity(u_int32_t x) {
    u_int32_t count = 0, i, b = 1;
    for (i = 0; i < 32; i++) {
        if (x & (b << i)) {
            count++;
        }
    }
    if ((count % 2)) {
        return 0;
    }
    return 1;
}

u_int32_t MilesTagTX::add_parity(u_int32_t x) {
    u_int32_t parity = 0;
    if (has_even_parity(x)) {
        parity = 1;
    }
    x = x << 1;
    x = x | parity;
    return x;
}

u_int32_t MilesTagTX::calculateChecksum(u_int32_t data, int nbits) {
    u_int32_t checksum = 0;
    for (int i = 0; i < nbits; i++) {
        if (data & (1UL << i)) {
            checksum++;
        }
    }
    return checksum;
}

// Recieve code
MilesTagRX::MilesTagRX() {
    if (DEBUG) Serial.print("ESP32_IR::Constructing");
}

MilesTagRX::MilesTagRX(int _txPin, int _channel) {
    if (DEBUG) Serial.print("ESP32_IR::Constructing");
    SetRx(_txPin, _channel);
}

bool MilesTagRX::SetRx(int _rxPin, int _channel) {
    bool _status = true;

    if (_rxPin >= GPIO_NUM_0 && _rxPin < GPIO_NUM_MAX)
        rxGpioNum = _rxPin;
    else
        _status = false;

    if (_channel >= RMT_CHANNEL_0 && _channel < RMT_CHANNEL_MAX)
        rxRmtPort = _channel;
    else
        _status = false;

    if (_status == false && DEBUG) Serial.println("ESP32_IR::Tx Pin init failed");
    return _status;
}

void MilesTagRX::rxConfig() {
    rmt_config_t config;
    config.rmt_mode = RMT_MODE_RX;
    config.channel = (rmt_channel_t)rxRmtPort;
    config.gpio_num = (gpio_num_t)rxGpioNum;
    gpio_pullup_en((gpio_num_t)rxGpioNum);
    config.mem_block_num = 2;  // how many memory blocks 64 x N (0-7)
    config.rx_config.filter_en = 1;
    config.rx_config.filter_ticks_thresh = 100;  // 80000000/100 -> 800000 / 100 = 8000  = 125us
    config.rx_config.idle_threshold = END_US + OFFSET;
    config.clk_div = CLK_DIV;
    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 1000, 0));
    // rmt_get_ringbuf_handle(config.channel, &ringBuf);
    rmt_rx_start(config.channel, 1);
}

void MilesTagRX::clearHits() {
    for (int i = 0; i < 20; i++) {
        Hits[i].playerId = 0;
        Hits[i].quantity = 0;
        Hits[i].error = true;
    }
    hitCount = 0;
}
void MilesTagRX::clearCommands() {
    for (int i = 0; i < 20; i++) {
        Commands[i].command = 0;
        Commands[i].data = 0;
        Commands[i].error = true;
    }
    commandCount = 0;
}

bool MilesTagRX::bufferPull() {
    u_int32_t data = 0;
    RingbufHandle_t ringBuf = NULL;
    rmt_config_t config;
    // Serial.println("RX Pin: " + String(rxGpioNum) + " Channel: " + String(rxRmtPort));
    config.channel = (rmt_channel_t)rxRmtPort;
    rmt_get_ringbuf_handle(config.channel, &ringBuf);

    if (ringBuf == NULL) {
        Serial.println("Failed to get Ring Buffer Handle.");
        return false;
    }

    // while(ringBuf) {
    size_t rx_size = 0;
    rmt_item32_t *item = (rmt_item32_t *)xRingbufferReceive(ringBuf, &rx_size, (TickType_t)TIMEOUT_US);
    int numItems = rx_size / sizeof(rmt_item32_t);
    if (numItems == 0) {
        return false;  // If no items, continue to next iteration
    }
    // memset(item, 0, sizeof(unsigned int) * 64);
    rmt_item32_t *itemproc = item;

    bool shotCommand = false;
    int bitNumber = 0;

    for (size_t i = 0; i < (rx_size / 4); i++) {
        // uint32_t cpuFreq = getCpuFrequencyMhz();
        // Serial.print("CPU frequency: ");
        // Serial.print(cpuFreq);
        // Serial.println(" MHz");
        // unsigned int markValue = (itemproc->duration0) / 0.0125;

        unsigned int markValue = (itemproc->duration0);
        if (markValue < 100) {
            markValue = (itemproc->duration0) / 0.0125;
        }
        markValue = ROUND_TO * round((float)markValue / ROUND_TO);

        // unsigned int spaceValue = (itemproc->duration1) / 0.0125;
        unsigned int spaceValue = (itemproc->duration1);
        if (spaceValue < 100) {
            spaceValue = (itemproc->duration1) / 0.0125;
        }
        spaceValue = ROUND_TO * round((float)spaceValue / ROUND_TO);

        if (markValue > (SHOT_HEADER_US - OFFSET) && markValue < (SHOT_HEADER_US + OFFSET)) {
            Serial.println("");
            Serial.println("**************************************************");
            // Serial.println("Header Received");
            Serial.println("Decode Shot (Short)");
            bitNumber = 0;
            data = 0;
            shotCommand = true;
        } else if (markValue > (CMD_HEADER_US - OFFSET) && markValue < (CMD_HEADER_US + OFFSET)) {
            Serial.println("");
            Serial.println("**************************************************");
            // Serial.println("Header Received");
            Serial.println("Decode Command (Long)");
            bitNumber = 0;
            data = 0;
            shotCommand = false;
        } else if (markValue > (CMD_EXTRA_US - OFFSET) && markValue < (CMD_EXTRA_US + OFFSET)) {
            Serial.println("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
            Serial.println("");
        } else if (markValue > (END_US - OFFSET) && markValue < (END_US + OFFSET)) {
            if (shotCommand == true) {
                Serial.println("Shot - " + String(data, BIN));
                Hits[hitCount] = decodeShotData(data, bitNumber);
                // Serial.println("Hitcount before: " + String(hitCount));
                hitCount++;
                // Serial.println("Hit Count - " + String(hitCount));
            } else {
                Serial.println("Command - " + String(data, BIN));

                Commands[commandCount] = decodeCommandData(data, bitNumber);
                commandCount++;
                Serial.println("Command Count - " + String(commandCount));
            }
        } else {
            // Loop through the bits
            if (markValue > (ONE_US - OFFSET) && markValue < (ONE_US + OFFSET)) {
                data = data << 1 | 1;
                // Serial.print("Bit: 1 - ");
                // Serial.print("Bit: q - " + String(data,BIN));
            } else if (markValue > (ZERO_US - OFFSET) && markValue < (ZERO_US + OFFSET)) {
                data = data << 1 | 0;
                // Serial.print("Bit: 0 - " + String(data,BIN));
            }
        }

        // Serial.println("Item " + String(bitNumber) + " - " + String(itemproc->duration0) + "("+String(markValue)+")" );

        bitNumber++;
        ++itemproc;
    }
    // Serial.println("bufferPull - " + String(data,HEX));

    // printBinary(data, 12);
    vRingbufferReturnItem(ringBuf, (void *)item);

    // Serial.println("**************************************************");
    if (commandCount > 0 || hitCount > 0) {
        return true;
    } else {
        return false;
    }

    // }
}

MTShotRecieved MilesTagRX::decodeShotData(u_int32_t data, uint8_t bitCount) {
    Serial.println("Decode Shot Data: " + String(data, BIN) + " - " + String(bitCount) + " bits");
    MTShotRecieved decodedData;

    u_int32_t dataWp = data >> 4;  // Shift 4 bits to remove the checksum

    decodedData.playerId = (dataWp & 0x3F);
    decodedData.quantity = binToQuantity((dataWp & 0x3C0) >> 6);

    decodedData.error = false;

    u_int32_t receivedChecksum = data & 0x0F;
    u_int32_t calculatedChecksum = calculateChecksum(dataWp, 10);

    if (receivedChecksum != calculatedChecksum) {
        Serial.println("Checksum Error" + String(receivedChecksum) + " - " + String(calculatedChecksum));
        decodedData.error = true;
    }

    decodedData.noOfBits = bitCount - 5;  // Remove the 4 checksum bits and 2 extra bits plus 1 because it start at 0
    if (decodedData.noOfBits != 10) {
        Serial.println("Bit Count - " + String(decodedData.noOfBits));
        decodedData.error = true;
    }
    if (decodedData.quantity > 100) {
        Serial.println("Quantity - " + String(decodedData.quantity));
        decodedData.error = true;
    }
    if (decodedData.playerId > 63) {
        Serial.println("Player ID - " + String(decodedData.playerId));
        decodedData.error = true;
    }

    return decodedData;
}

MTCommandData MilesTagRX::decodeCommandData(u_int32_t data, uint8_t bitCount) {
    Serial.println("Decode Command Data: " + String(data, BIN) + " - " + String(bitCount) + " bits");
    MTCommandData decodedData;

    u_int32_t dataWp = data >> 4;  // Shift 4 bits to remove the checksum

    decodedData.command = (dataWp & 0xF0000) >> 16;
    Serial.println("Command - " + String(decodedData.command, BIN));

    decodedData.data = (dataWp & 0x0FFFF);
    Serial.println("Data - " + String(decodedData.data, BIN));

    decodedData.error = false;

    u_int32_t receivedChecksum = data & 0x0F;

    u_int32_t calculatedChecksum = calculateChecksum(dataWp, 20);

    if (receivedChecksum != calculatedChecksum) {
        Serial.println("Checksum Error" + String(receivedChecksum, BIN) + " - " + String(calculatedChecksum, BIN));
        decodedData.error = true;
    }
    decodedData.noOfBits = bitCount - 5;  // Remove the 4 checksum bits and 2 extra bits plus 1 because it start at 0

    if (decodedData.noOfBits != 20) {
        Serial.println("Bit Count - " + String(decodedData.noOfBits));
        decodedData.error = true;
    }
    if (decodedData.command > 0xf) {
        Serial.println("Command - " + String(decodedData.command));
        decodedData.error = true;
    }
    if (decodedData.data > 0xffff) {
        Serial.println("Data - " + String(decodedData.data));
        decodedData.error = true;
    }

    return decodedData;
}

void MilesTagRX::processCommand(uint16_t command) {
    // Process the command according to your game logic
    // For example, you can use a switch statement to handle different commands
    switch (command) {
        case 0x01:  // Example command
            // Handle command 0x01
            break;
        // ... add more cases for other commands
        default:
            // Handle unrecognized command
            break;
    }
}

u_int32_t MilesTagRX::binToQuantity(u_int32_t dmg) {
    // Serial.println("Dmg - " + String(dmg));
    u_int32_t dmgarray[16] = {1, 2, 4, 5, 7, 10, 15, 17, 20, 25, 30, 35, 40, 50, 75, 100};
    return dmgarray[dmg];
}

u_int32_t MilesTagRX::has_even_parity(u_int32_t x) {
    u_int32_t count = 0, i, b = 1;
    for (i = 0; i < 32; i++) {
        if (x & (b << i)) {
            count++;
        }
    }
    if ((count % 2)) {
        return 0;
    }
    return 1;
}

u_int32_t MilesTagRX::extractChecksum(u_int32_t data, int nbits) {
    u_int32_t checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum |= ((data >> (nbits + i)) & 1UL) << (3 - i);
    }
    return checksum;
}

u_int32_t MilesTagRX::calculateChecksum(u_int32_t data, int nbits) {
    u_int32_t checksum = 0;
    for (int i = 0; i < nbits; i++) {
        if (data & (1UL << i)) {
            checksum++;
        }
    }
    return checksum;
}