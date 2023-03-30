// Crossfire constants provided by Team Black Sheep under terms of the 2-Clause BSD License

#ifndef CRSF_H
#define CRSF_H

#include <cstdint>

#define CRSF_BAUDRATE   420000

// Frame Format:
// 420k baud default
// 8 bit
// 1 stop bit
// Frame format:
//    <address><frame length><type><payload><crc>


// enums / globals
enum { CRSF_SYNC_BYTE = 0xC8 };

enum { CRSF_FRAME_SIZE_MAX = 64 }; // 62 bytes frame plus 2 bytes frame header(<length><type>)
enum { CRSF_PAYLOAD_SIZE_MAX = CRSF_FRAME_SIZE_MAX - 6 };

enum CRSFFrameType {
  CRSF_FRAMETYPE_GPS = 0x02,
  CRSF_FRAMETYPE_BATTERY_SENSOR = 0x08,
  CRSF_FRAMETYPE_HEARTBEAT = 0x0B,
  CRSF_FRAMETYPE_LINK_STATISTICS = 0x14,
  CRSF_FRAMETYPE_RC_CHANNELS_PACKED = 0x16,
  CRSF_FRAMETYPE_SUBSET_RC_CHANNELS_PACKED = 0x17,
  CRSF_FRAMETYPE_LINK_STATISTICS_RX = 0x1C,
  CRSF_FRAMETYPE_LINK_STATISTICS_TX = 0x1D,
  CRSF_FRAMETYPE_ATTITUDE = 0x1E,
  CRSF_FRAMETYPE_FLIGHT_MODE = 0x21,
  // Extended Header Frames, range: 0x28 to 0x96
  CRSF_FRAMETYPE_DEVICE_PING = 0x28,
  CRSF_FRAMETYPE_DEVICE_INFO = 0x29,
  CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY = 0x2B,
  CRSF_FRAMETYPE_PARAMETER_READ = 0x2C,
  CRSF_FRAMETYPE_PARAMETER_WRITE = 0x2D,
  CRSF_FRAMETYPE_COMMAND = 0x32,
  // MSP commands
  CRSF_FRAMETYPE_MSP_REQ = 0x7A,   // response request using msp sequence as command
  CRSF_FRAMETYPE_MSP_RESP = 0x7B,  // reply with 58 byte chunked binary
  CRSF_FRAMETYPE_MSP_WRITE = 0x7C,  // write with 8 byte chunked binary (OpenTX outbound telemetry buffer limit)
  CRSF_FRAMETYPE_DISPLAYPORT_CMD = 0x7D, // displayport control command
  // Error
  CRSF_FRAMETYPE_ERROR = 0x00,  // Invalid or unsupported frame type, or corrupt data
};

enum {
  CRSF_COMMAND_SUBCMD_GENERAL = 0x0A,    // general command
};

enum {
  CRSF_COMMAND_SUBCMD_GENERAL_CRSF_SPEED_PROPOSAL = 0x70,    // proposed new CRSF port speed
  CRSF_COMMAND_SUBCMD_GENERAL_CRSF_SPEED_RESPONSE = 0x71,    // response to the proposed CRSF port speed
};

enum {
  CRSF_DISPLAYPORT_SUBCMD_UPDATE = 0x01, // transmit displayport buffer to remote
  CRSF_DISPLAYPORT_SUBCMD_CLEAR = 0X02, // clear client screen
  CRSF_DISPLAYPORT_SUBCMD_OPEN = 0x03,  // client request to open cms menu
  CRSF_DISPLAYPORT_SUBCMD_CLOSE = 0x04,  // client request to close cms menu
  CRSF_DISPLAYPORT_SUBCMD_POLL = 0x05,  // client request to poll/refresh cms menu
};

enum {
  CRSF_DISPLAYPORT_OPEN_ROWS_OFFSET = 1,
  CRSF_DISPLAYPORT_OPEN_COLS_OFFSET = 2,
};

enum {
  CRSF_FRAME_GPS_PAYLOAD_SIZE = 15,
  CRSF_FRAME_BATTERY_SENSOR_PAYLOAD_SIZE = 8,
  CRSF_FRAME_HEARTBEAT_PAYLOAD_SIZE = 2,
  CRSF_FRAME_LINK_STATISTICS_PAYLOAD_SIZE = 10,
  CRSF_FRAME_LINK_STATISTICS_TX_PAYLOAD_SIZE = 6,
  CRSF_FRAME_RC_CHANNELS_PAYLOAD_SIZE = 22, // 11 bits per channel * 16 channels = 22 bytes.
  CRSF_FRAME_ATTITUDE_PAYLOAD_SIZE = 6,
  CRSF_FRAME_DEVICE_PING_PAYLOAD_SIZE = 2,
};

enum {
  CRSF_FRAME_LENGTH_ADDRESS = 1, // length of ADDRESS field
  CRSF_FRAME_LENGTH_FRAMELENGTH = 1, // length of FRAMELENGTH field
  CRSF_FRAME_LENGTH_TYPE = 1, // length of TYPE field
  CRSF_FRAME_LENGTH_CRC = 1, // length of CRC field
  CRSF_FRAME_LENGTH_TYPE_CRC = 2, // length of TYPE and CRC fields combined
  CRSF_FRAME_LENGTH_EXT_TYPE_CRC = 4, // length of Extended Dest/Origin, TYPE and CRC fields combined
  CRSF_FRAME_LENGTH_NON_PAYLOAD = 4, // combined length of all fields except payload
};

enum {
  CRSF_FRAME_TX_MSP_FRAME_SIZE = 58,
  CRSF_FRAME_RX_MSP_FRAME_SIZE = 8,
  CRSF_FRAME_ORIGIN_DEST_SIZE = 2,
};

// // Clashes with CRSF_ADDRESS_FLIGHT_CONTROLLER
// #define CRSF_SYNC_BYTE 0XC8

typedef enum {
  CRSF_ADDRESS_BROADCAST = 0x00,
  CRSF_ADDRESS_USB = 0x10,
  CRSF_ADDRESS_TBS_CORE_PNP_PRO = 0x80,
  CRSF_ADDRESS_RESERVED1 = 0x8A,
  CRSF_ADDRESS_CURRENT_SENSOR = 0xC0,
  CRSF_ADDRESS_GPS = 0xC2,
  CRSF_ADDRESS_TBS_BLACKBOX = 0xC4,
  CRSF_ADDRESS_FLIGHT_CONTROLLER = 0xC8,
  CRSF_ADDRESS_RESERVED2 = 0xCA,
  CRSF_ADDRESS_RACE_TAG = 0xCC,
  CRSF_ADDRESS_RADIO_TRANSMITTER = 0xEA,
  CRSF_ADDRESS_CRSF_RECEIVER = 0xEC,
  CRSF_ADDRESS_CRSF_TRANSMITTER = 0xEE
};

enum CRSFFrameStatus {
  CRSF_FRAME_VALID = 1,
  CRSF_FRAME_ERROR_ADDRESS = -1,
  CRSF_FRAME_ERROR_PAYLOAD = -2,
  CRSF_FRAME_ERROR_TYPE = -3,
};

// CRC-8/DVB-S2 Lookup Table
static uint8_t crc8_dvbs2[256] = {
  0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54, 0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
  0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06, 0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
  0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0, 0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
  0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2, 0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
  0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9, 0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
  0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B, 0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
  0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D, 0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
  0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F, 0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
  0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB, 0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
  0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9, 0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
  0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F, 0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
  0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D, 0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
  0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26, 0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
  0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74, 0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
  0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82, 0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
  0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0, 0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9
};

// Function declaration
uint8_t crsf_crc(const uint8_t* buf, uint8_t len);


// Class declaration
class CRSFInterface {
  CRSFInterface() {}
  
  //bool readFrame(bool (*callback)());
  CRSFFrameStatus decodeFrame(uint8_t* buf, unsigned int len, CRSFFrameType* type_rtn = nullptr);
  bool getChannels(unsigned int (&channels)[16])const;
  bool getChannel(unsigned int ch, unsigned int& value);
  //bool writeTelemetry(bool (*callback)());
  unsigned int getBaudRate() { return baud_rate; }

private:
  unsigned int baud_rate = CRSF_BAUDRATE;
  unsigned int rc_channels[16] = {0};

  bool unpackRCChannels(uint8_t* payload, unsigned int channel_count, unsigned int bits);
};

#endif
