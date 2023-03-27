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

CRSFFrameStatus CRSFInterface::decodeFrame(uint8_t* buf, unsigned int len, CRSFFrameType* type_rtn = nullptr) {
  uint8_t addr = buf[0];
  uint8_t frame_length = buf[1];  // Includes type, but not addr/crc
  uint8_t payload_size = frame_length - 1;
  uint8_t type = buf[2];
  uint8_t crc = buf[len - 1];

  if (type_rtn)
    *type_rtn = CRSF_FRAMETYPE_ERROR;

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

      if (decodeRCChannels(&buf[3], 16, 11))
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

// Connor's version
// void unpack_channels(uint8_t* data, size_t bits_per_channel, uint16_t* channels, size_t num_channels) {
//     for (size_t i_ch = 0; i_ch < num_channels; i_ch++) {
//         channels[i_ch] = 0;
//         size_t i_bit = i_ch * bits_per_channel;
//         for (size_t channel_bit = 0; channel_bit < bits_per_channel;) {
//             size_t read_byte = (i_bit + channel_bit) / 8;
//             size_t read_byte_bit = (i_bit + channel_bit) % 8;
//             size_t read_byte_bits = 8 - read_byte_bit;
//             if (channel_bit + read_byte_bits > bits_per_channel) {
//                 read_byte_bits = bits_per_channel - channel_bit;
//             }
//             uint8_t masked_byte = (data[read_byte] >> (8 - read_byte_bits - read_byte_bit)) & ((1 << read_byte_bits) - 1);
//             channels[i_ch] |= masked_byte << (bits_per_channel - read_byte_bits - channel_bit);
//             channel_bit += read_byte_bits;
//         }
//     }
// }