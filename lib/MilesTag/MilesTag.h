/*
Copyright <2018> <Ethan Johnston>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef MilesTag_h
#define MilesTag_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp32-hal.h"
// #include "esp_intr.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "driver/rmt.h"
#include "esp_intr_alloc.h"
#include "freertos/semphr.h"
#include "soc/rmt_struct.h"

#ifdef __cplusplus
}

#endif

// #include <map>

// // Structure to hold command information
// struct Command {
//   String description;
//   byte firstByte;
//   byte secondByte;
//   byte rangeEndByte;
// };

// // Create a map with command names mapped to their details
// std::map<String, Command> commandMap = {
//   { "AddHealth", {"Add Health 1 to 100", 0x80, 0x01, 0xE8} },
//   { "AddRounds", {"Add Rounds 1 to 100", 0x81, 0x01, 0xE8} },
//   { "RESERVED_82", {"RESERVED_82", 0x82,0x00,0x00}},
//   { "AdminKill", {"Admin Kill", 0x83, 0x00, 0xE8} },
//   { "PauseUnpause", {"Pause/Unpause", 0x83, 0x01, 0xE8} },
//   { "StartGame", {"Start Game", 0x83, 0x02, 0xE8} },
//   { "RestoreDefaults", {"Restore Defaults", 0x83, 0x03, 0xE8} },
//   { "Respawn", {"Respawn", 0x83, 0x04, 0xE8} },
//   { "NewGame", {"New Game", 0x83, 0x05, 0xE8} },
//   { "FullAmmo", {"Full Ammo", 0x83, 0x06, 0xE8} },
//   { "EndGame", {"End Game", 0x83, 0x07, 0xE8} },
//   { "ResetClock", {"Reset Clock", 0x83, 0x08, 0xE8} },
//   { "RESERVED_83_09", {"RESERVED_84_09", 0x83,0x09,0x00}},
//   { "InitializePlayer", {"Initialize Player", 0x83, 0x0A, 0xE8} },
//   { "ExplodePlayer", {"Explode Player", 0x83, 0x0B, 0xE8} },
//   { "NewGameReady", {"New Game (Ready)", 0x83, 0x0C, 0xE8} },
//   { "FullHealth", {"Full Health", 0x83, 0x0D, 0xE8} },
//   { "RESERVED_83_0E", {"RESERVED_84_0E", 0x83,0x0E,0x00}},
//   { "FullArmor", {"Full Armor", 0x83, 0x0F, 0xE8} },
//   { "RESERVED_83_10", {"RESERVED_84_10", 0x83,0x10,0x00}},
//   { "RESERVED_83_11", {"RESERVED_84_11", 0x83,0x11,0x00}},
//   { "RESERVED_83_12", {"RESERVED_84_12", 0x83,0x12,0x00}},
//   { "RESERVED_83_13", {"RESERVED_84_13", 0x83,0x13,0x00}},
//   { "ClearScores", {"Clear Scores", 0x83, 0x14, 0xE8} },
//   { "TestSensors", {"Test Sensors", 0x83, 0x15, 0xE8} },
//   { "StunPlayer", {"Stun Player", 0x83, 0x16, 0xE8} },
//   { "DisarmPlayer", {"Disarm Player", 0x83, 0x17, 0xE8} },
//   { "RESERVED_84", {"RESERVED_84", 0x84,0x00,0x00}},
//   { "RESERVED_85", {"RESERVED_85", 0x85,0x00,0x00}},
//   { "RESERVED_86", {"RESERVED_86", 0x86,0x00,0x00}},
//   { "SystemData_87_00", {"System Data 87 Reserved 00", 0x87,0x00,0x00}},
//   { "CloningData", {"Cloning Data", 0x87,0x01,0x00}},
//   { "ScoreData1", {"Score Data (pt 1)", 0x87,0x03,0x00}},                                                         //This scoring data provided in this packet shows all the player specific data.
//   { "ScoreData2", {"Score Data (pt 2)", 0x87,0x04,0x00}},                                                         //This scoring data provided in this packet shows all hits that were recieved by the player.
//   { "ScoreData3", {"Score Data (pt 3)", 0x87,0x05,0x00}},                                                         //This scoring data provided in this packet shows hits from the same colour team (traitor shots) that were recieved by the
//   player. { "RESERVED_88", {"RESERVED_88", 0x88,0x00,0x00}}, { "RESERVED_89", {"RESERVED_89", 0x89,0x00,0x00}}, { "PickupClipsBox", {"Pickup Clips Box, quantity 0 to 15", 0x8A, 0x00, 0xE8} }, { "PickupHealthBox", {"Pickup Health Box,
//   quantity 0 to 15", 0x8B, 0x00, 0xE8} }, { "FlagPickup", {"Flag Pickup, Flag ID 0 to 15", 0x8C, 0x00, 0xE8} }, { "RadiationZoneRemoveHealth", {"Radiation Zone remove Health 1 to 100", 0xA0,0x00, 0xE8} }, { "AnomalyZoneAddHealth",
//   {"Anomaly Zone Add Health 1 to 100", 0xA1, 0x01, 0xE8} }, { "SetGameMode", {"Set Game Mode", 0xA8, 0x00, 0xE8} }, { "SetTeamID", {"Set Team ID 0 to 3", 0xA9, 0x00, 0xE8} }, { "AddOrRemoveRedMemberHealth", {"Add or Remove Red member
//   Health -128 to 127", 0xB0, 0x00, 0xE8} }, { "AddOrRemoveBlueMemberHealth", {"Add or Remove Blue member Health -128 to 127", 0xB1, 0x00, 0xE8} }, { "AddOrRemoveYellowMemberHealth", {"Add or Remove Yellow member Health -128 to 127",
//   0xB2, 0x00, 0xE8} }, { "AddOrRemoveGreenMemberHealth", {"Add or Remove Green member Health -128 to 127", 0xB3, 0x00, 0xE8} }, { "RespawnTeamMember", {"Respawn Team member ID 0 to 4", 0xB4, 0x00, 0xE8} }, { "KillTeamMember", {"Kill Team
//   member ID 0 to 4", 0xB6, 0x00, 0xE8} }, { "FullAmmoTeamMember", {"Full Ammo Team member ID 0 to 4", 0xB7, 0x00, 0xE8} }
// };

// For cloning, much more data is required to be sent. A further 36 bytes is sent
// after the 0xE8 byte to mark the end of the message.
// This table starts from byte 4, which is the first byte sent after the 0xE8.

// Cloning data
//  Byte number Description of data
//  4 Reserved
//  5 Reserved
//  6 Reserved
//  7 Team ID - See section 2.3.1
//  8 Reserved
//  9 Clips added by picking up an ammo box
//  10 Health added by picking up a medic box
//  11 Reserved (0x03 for 5.41)
//  12 Hit LED timeout in seconds
//  13 Sound set - See section 2.3.2
//  14 Overheat limit in rounds/min
//  15 Reserved
//  16 Reserved
//  17 Damage per shot - See section 2.3.3
//  18 Clip size - 0xFF is unlimited
//  19 Number of clips - 0xCA is unlimited
//  20 Fire selector - See 2.3.4
//  21 Number of rounds for burst mode
//  22 Cyclic RPM - See 2.3.5
//  23 Reload delay in seconds
//  24 IR power - See 2.3.6
//  25 IR range - See 2.3.7
//  26 Tagger on/off settings - See 2.3.8
//  27 Respawn health - See 2.3.9
//  28 Reserved
//  29 Respawn delay in tens of seconds
//  30 Armour value
//  31 Game on/off settings 1/2 - See 2.3.10
//  32 Game on/off settings 2/2 - See 2.3.11
//  33 Hit delay - See 2.3.12
//  34 Start delay in seconds
//  35 Death delay in seconds
//  36 Time limit in minutes
//  37 Maximum respawns
//  38 Reserved (0xFF in 5.41)
//  39 Checksum - See 2.3.13

// // Byte 7 - Team ID
// std::map<uint8_t, std::string> teamMap = {
//     {0x00, "Red"},
//     {0x01, "Blue"},
//     {0x02, "Yellow"},
//     {0x03, "Green"}
// };

// // Byte 13 - Sound set
// std::map<uint8_t, std::string> soundSetMap = {
//     {0x00, "Mil-sim"},
//     {0x01, "Sci-fi"},
//     {0x02, "Silenced"}
// };

// // Byte 17 - Damage per shot
// std::map<uint8_t, int> damageMap = {
//     {0x00, 1},
//     {0x01, 2},
//     {0x02, 4},
//     {0x03, 5},
//     {0x04, 7},
//     {0x05, 10},
//     {0x06, 15},
//     {0x07, 17},
//     {0x08, 20},
//     {0x09, 25},
//     {0x0A, 30},
//     {0x0B, 35},
//     {0x0C, 40},
//     {0x0D, 50},
//     {0x0E, 75},
//     {0x0F, 100}
// };

// // Fire selector - Byte 20
// std::map<uint8_t, std::string> fireModeMap = {
//     {0x00, "Semi Auto"},
//     {0x01, "Burst"},
//     {0x02, "Full Auto"}
// };

// //Cyclic RPM - Byte 22
// std::map<uint8_t, int> cyclicRpmMap = {
//     {0x00, 250},
//     {0x01, 300},
//     {0x02, 350},
//     {0x03, 400},
//     {0x04, 450},
//     {0x05, 500},
//     {0x06, 550},
//     {0x07, 600},
//     {0x08, 650},
//     {0x09, 700},
//     {0x0A, 750},
//     {0x0B, 800}
// };

// // IR Power - Byte 25
// std::map<uint8_t, std::string> irPowerMap = {
//     {0x00, "Indoors"},
//     {0x01, "Outdoors"},
//     {0x02, "Maximum"}
// };

// // IR Range - Byte 25
// std::map<uint8_t, std::string> irRangeMap = {
//     {0x01, "Minimum"},
//     {0x02, "10%"},
//     {0x04, "20%"},
//     {0x07, "40%"},
//     {0x0A, "60%"},
//     {0x0E, "80%"},
//     {0x12, "Maximum"}
// };

// // tagger Settings - Byte 26
// std::map<uint8_t, std::string> taggerSettingsMap = {
//     {0x01, "Muzzle flash on"},
//     {0x02, "Overheat feature on"},
//     {0x04, ""},
//     {0x08, ""},
//     {0x10, ""},
//     {0x20, ""},
//     {0x40, ""},
//     {0x80, ""}
// };

// // Respawn health - Byte 27
// std::array<uint8_t, 56> respawnHealthArray = {
//     1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
//     25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100,
//     105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160, 165,
//     170, 175, 180, 185, 190, 195, 200
// };

// // Game Settings - Byte 31
// std::map<uint8_t, std::string> gameSettingsMap2 = {
//     {0x01, "Hit LED enabled"},
//     {0x02, "Friendly fire enabled"},
//     {0x04, "Unlimited clips enabled"},
//     {0x08, "Zombie mode enabled"},
//     {0x10, "Medics enabled"},
//     {0x20, "Game boxes reset on respawn"},
//     {0x40, "Game boxes are not used up"}
// };

// // Game Settings - Byte 32
// std::map<uint8_t, std::string> gameSettingsMap3 = {
//   {0x01, "Capture-the-flag display enabled"},
//   {0x02, "Respawn enabled"},
//   {0x04, "Tagger nicknames enabled"},
//   {0x08, "Old IR level field"},
//   {0x10, "Full ammo reset on respawn"},
//   {0x20, "Enable game mode"}
// };

// Game Settings - Byte 33
// std::array<float, 24> hitDelay = {0.00, 0.25, 0.5, 0.75, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
// uint8_t byteValue = 0x0A; // or any valid value
// float delay = hitDelay[byteValue]; // this will be 7.0 for byteValue = 0x0A

// struct PlayerData {
//   uint8_t playerId;
//   uint8_t teamId; // refer to section 2.3.1 for interpretation
//   uint16_t roundsFired; // note: big endian
//   uint16_t totalHits; // note: big endian
//   uint8_t gameTimeMinutes;
//   uint8_t gameTimeSeconds;
//   uint8_t respawns;
//   uint8_t taggedOutCounter;
//   uint8_t flagCounter;
//   uint8_t checksum; // refer to section 2.3.13 for calculation
// };

class MilesTagTX {
   public:
    MilesTagTX();
    MilesTagTX(int _txPin, int _channel);
    bool SetTx(int _txPin, int _channel);
    void txConfig();
    void fireShot(u_int32_t playerId, u_int32_t dmg);
    void sendCommand(bool shotCommand, uint8_t command, uint64_t data);
    void sendIR(rmt_item32_t data[], int IRlength, bool waitTilDone);

   private:
    void irTransmit(bool shotCommand, bool extraData, u_int32_t Buffer, int nbits);
    u_int32_t quantitytoBin(u_int32_t dmg);
    u_int32_t has_even_parity(u_int32_t x);
    u_int32_t add_parity(u_int32_t x);
    u_int32_t calculateChecksum(u_int32_t data, int nbits);

    int txGpioNum;
    int txRmtPort;
};

typedef struct MTShotRecieved {
    u_int32_t noOfBits;
    u_int32_t quantity;
    u_int32_t playerId;
    bool error = true;
} MTShotRecieved;

typedef struct {
    u_int32_t noOfBits;
    uint8_t command;
    uint32_t data;
    bool error;
} MTCommandData;

class MilesTagRX {
   public:
    MilesTagRX();
    MilesTagRX(int _rxPin, int _channel);
    bool SetRx(int _rxPin, int _channel);
    void rxConfig();
    int readIR();
    void decodeRAW(rmt_item32_t *rawDataIn, int numItems, unsigned int *irDataOut);
    void getDataIR(rmt_item32_t item, unsigned int *irDataOut, int index);
    MTShotRecieved decodeShotData(u_int32_t data, uint8_t bitCount);
    MTCommandData decodeCommandData(u_int32_t data, uint8_t bitCount);
    void processCommand(uint16_t command);
    void clearHits();
    void clearCommands();
    bool bufferPull();
    MTShotRecieved Hits[20];
    MTCommandData Commands[20];
    int hitCount = 0;
    int commandCount = 0;

   private:
    u_int32_t binToQuantity(u_int32_t dmg);
    u_int32_t has_even_parity(u_int32_t x);
    u_int32_t extractChecksum(u_int32_t data, int nbits);
    u_int32_t calculateChecksum(u_int32_t data, int nbits);
    int rxGpioNum;
    int rxRmtPort;
};
#endif