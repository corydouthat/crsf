#include "crsf.h"

// CRC-8/DVB-S2
// Using lookup table
uint8_t crsf_crc(const uint8_t* buf, uint8_t len) {
  uint8_t crc = 0;

  for (uint8_t i = 0; i < len - 1; i++) {
    crc = crc8_dvbs2[crc ^ *buf++];
  }

  return crc;
}

CRSFFrameStatus CRSFInterface::decodeFrame(uint8_t* buf, unsigned int full_frame_length, CRSFFrameType* type_rtn) {
  uint8_t addr = buf[0];
  uint8_t frame_length = buf[1];  // Includes type+crc, but not addr/len
  uint8_t payload_size = frame_length - 2;
  uint8_t type = buf[2];
  uint8_t crc = buf[full_frame_length - 1];

  if (type_rtn)
    *type_rtn = CRSF_FRAMETYPE_ERROR;

  if (addr != 0xC8)
    return CRSF_FRAME_ERROR_ADDRESS;

  if (frame_length < 5 || full_frame_length > CRSF_FRAME_SIZE_MAX || frame_length != full_frame_length - 2) {
    printf("CRSF Decode ERROR: payload/frame size\n");
    // printf("full_frame_length = %u\n", full_frame_length);
    // printf("frame_length = %u\n", frame_length);
    // printf("payload_size = %u\n", payload_size);
    // printf("last byte: %u", buf[full_frame_length - 1]);
    return CRSF_FRAME_ERROR_PAYLOAD;
  }

  if (crsf_crc(&buf[2], frame_length) != crc) {
    printf("CRSF Decode ERROR: payload CRC\n");
    return CRSF_FRAME_ERROR_PAYLOAD;
  }

  switch (type) {
    case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
      // 16 channels x 11 bits = 22 bytes
      if (payload_size != CRSF_FRAME_RC_CHANNELS_PAYLOAD_SIZE)
        return CRSF_FRAME_ERROR_PAYLOAD;

      if (unpackRCChannels(&buf[3], 16, 11))
      {
        *type_rtn = CRSF_FRAMETYPE_RC_CHANNELS_PACKED;
        return CRSF_FRAME_VALID;
      }
      else
        return CRSF_FRAME_ERROR_PAYLOAD;

      break;

    default:
      // Currently only supporting RC channels packed
      return CRSF_FRAME_ERROR_TYPE;
  }
}

bool CRSFInterface::getChannels(unsigned int (&channels)[16])const {
  // TODO return false if channels not valid

  for (unsigned int i = 0; i < 16; i++)
    channels[i] = rc_channels[i];

  return true;
}

bool CRSFInterface::getChannel(unsigned int ch, unsigned int& value) {
  // TODO return false if channels not valid

  if (ch > 15)
    return false;

  value = rc_channels[ch];

  return true;
}

bool CRSFInterface::unpackRCChannels(uint8_t* data, unsigned int channel_count, unsigned int bits) {
  // TODO: check for valid payload size and number of bits
  
  if (!data)
    return false;

  if (bits > 16)
    return false;

  unsigned int byte_start = 0;
  unsigned int bit_start = 0;

  uint16_t temp;

  // Extract channels
  for (unsigned int ch = 0; ch < channel_count; ch++) {
    // Concatenate bytes
    if (byte_start < (channel_count * bits / 8 - 1))
      temp = (data[byte_start] << 8) | data[byte_start+1];
    else 
      temp = (data[byte_start] << 8) | 0x00;
    
    // Bit shift to get relevant data in least significant bits
    temp = temp << bit_start;
    temp = temp >> (16 - bits);
    rc_channels[ch] = temp;

    // Track where we're at
    bit_start += bits;
    byte_start += bit_start / 8;
    bit_start = bit_start % 8;
  }

  return true;
}
