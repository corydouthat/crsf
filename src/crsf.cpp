#include "crsf.h"

uint8_t flip_bits(uint8_t byte) {
  uint8_t temp = 0;
  uint8_t mask = 128;

  for (unsigned int i = 0; i < 8; i++) {
    if (byte & mask) {
      temp = temp | (0b1 << i);
    }
    mask = mask >> 1;
  }

  return temp;
}

// CRC-8/DVB-S2
// Using lookup table
uint8_t crsf_crc(const uint8_t* buf, uint8_t len) {
  uint8_t crc = 0;

  for (uint8_t i = 0; i < len - 1; i++) {
    crc = crc8_dvbs2[crc ^ *buf++];
  }

  return crc;
}

CRSFFrameStatus CRSFInterface::decodeFrame(const uint8_t* buf, unsigned int full_frame_length, unsigned int current_ms, CRSFFrameType* type_rtn) {
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

      if (unpackRCChannels(&buf[3], CRSF_CHANNEL_COUNT, CRSF_PACKED_BITS))
      {
        *type_rtn = CRSF_FRAMETYPE_RC_CHANNELS_PACKED;
        last_update_ms = current_ms;
        return CRSF_FRAME_VALID;
      }
      else
        return CRSF_FRAME_ERROR_PAYLOAD;

      break;

    default:
      // Currently only supporting RC channels packed
      //printf("CRSF Decode ERROR: unsupported frame type\n");
      return CRSF_FRAME_ERROR_TYPE;
  }
}

bool CRSFInterface::getChannel(unsigned int ch, unsigned int& value)const {
  // TODO return false if channels not valid

  if (ch > 15)
    return false;

  value = rc_channels[ch];

  return true;
}

bool CRSFInterface::unpackRCChannels(const uint8_t* data, unsigned int channel_count, unsigned int bits) {
  // TODO: check for valid payload size and number of bits
  
  if (!data)
    return false;

  if (bits > 16)
    return false;

  if (bits == 11 && channel_count == 16) {
    rc_channels[0]  = uint16_t((data[3-3]       | data[4-3] << 8) & 0x07FF);
    rc_channels[1]  = uint16_t((data[4-3] >> 3  | data[5-3] << 5) & 0x07FF);
    rc_channels[2]  = uint16_t((data[5-3] >> 6  | data[6-3] << 2 | data[7-3] << 10) & 0x07FF);
    rc_channels[3]  = uint16_t((data[7-3] >> 1  | data[8-3] << 7) & 0x07FF);
    rc_channels[4]  = uint16_t((data[8-3] >> 4  | data[9-3] << 4) & 0x07FF);
    rc_channels[5]  = uint16_t((data[9-3] >> 7  | data[10-3] << 1 | data[11-3] << 9) & 0x07FF);
    rc_channels[6]  = uint16_t((data[11-3] >> 2 | data[12-3] << 6) & 0x07FF);
    rc_channels[7]  = uint16_t((data[12-3] >> 5 | data[13-3] << 3) & 0x07FF);
    rc_channels[8]  = uint16_t((data[14-3]      | data[15-3] << 8) & 0x07FF);
    rc_channels[9]  = uint16_t((data[15-3] >> 3 | data[16-3] << 5) & 0x07FF);
    rc_channels[10] = uint16_t((data[16-3] >> 6 | data[17-3] << 2 | data[18-3] << 10) & 0x07FF);
    rc_channels[11] = uint16_t((data[18-3] >> 1 | data[19-3] << 7) & 0x07FF);
    rc_channels[12] = uint16_t((data[19-3] >> 4 | data[20-3] << 4) & 0x07FF);
    rc_channels[13] = uint16_t((data[20-3] >> 7 | data[21-3] << 1 | data[22-3] << 9) & 0x07FF);
    rc_channels[14] = uint16_t((data[22-3] >> 2 | data[23-3] << 6) & 0x07FF);
    rc_channels[15] = uint16_t((data[23-3] >> 5 | data[24-3] << 3) & 0x07FF);
  }
  else
    return false; // TODO: re-implement arbitrary bits with correct (reversed) byte ordering and bit ordering?

  // unsigned int byte_start = 0;
  // unsigned int bit_start = 0;

  // uint16_t temp;

  // // Extract channels
  // for (unsigned int ch = 0; ch < channel_count; ch++) {
  //   // Concatenate bytes
  //   if (byte_start < (channel_count * bits / 8 - 1))
  //     temp = (data[byte_start] << 8) | data[byte_start+1];
  //   else 
  //     temp = (data[byte_start] << 8) | 0x00;
    
  //   // Bit shift to get relevant data in least significant bits
  //   temp = temp << bit_start;
  //   temp = temp >> (16 - bits);
  //   rc_channels[ch] = temp;

  //   // Track where we're at
  //   bit_start += bits;
  //   byte_start += bit_start / 8;
  //   bit_start = bit_start % 8;
  // }

  return true;
}
