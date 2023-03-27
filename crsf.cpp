#include "crsf.h"

// CRC-8/DVB-S2
// Using lookup table
uint8_t crsf_crc(const uint8_t* buf, uint8_t len) {
  uint8_t crc = 0;

  for (uint8_t i = 0; i < len; i++) {
    crc = crc8_dvbs2[crc ^ *buf++];
  }

  return crc;
}

// Returns frame type
CRSFFrameType CRSFInterface::decodeFrame(uint8_t* buf, unsigned int len) {
  uint8_t addr = buf[0];
  uint8_t frame_length = buf[1];  // Includes type, but not addr/crc
  uint8_t payload_size = frame_length - 1;
  uint8_t type = buf[2];
  uint8_t crc = buf[len - 1];

  if (addr != 0xC8)
    return CRSF_FRAME_ERROR_ADDRESS;

  if (frame_length < 2 || frame_length + 2 > CRSF_FRAME_SIZE_MAX || frame_length != len - 2)
    return CRSF_FRAME_ERROR_PAYLOAD;

  if (crsf_crc(&buf[2], frame_length) != crc)
    return CRSF_FRAME_ERROR_PAYLOAD;

  switch (type) {
    case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
      // 16 channels x 11 bits = 22 bytes
      if (payload_size != CRSF_FRAME_RC_CHANNELS_PAYLOAD_SIZE)
        return CRSF_FRAME_ERROR_PAYLOAD;

      return decodeRCChannels(&buf[3], 16, 11);

      break;

    default:
      // Currently only supporting RC channels packed
      return CRSF_FRAME_ERROR_TYPE;
  }
}

bool CRSFInterface::decodeRCChannels(uint8_t* payload, unsigned int channel_count, unsigned int bits) {
  // TODO: check for valid payload size and number of bits

  unsigned int byte_start = 0;
  unsigned int bit_start = 0;

  for (unsigned int ch = 0; ch < channel_count; ch++) {
    rc_channels[ch] = (payload[byte_start]|payload[byte_start+1]<<bit_start) & uint8_t(bits * 8 - 1);

    bit_start += bits;
    byte_start += bit_start / bits;
    bit_start = bit_start % bits;
  }

  return true;
}
