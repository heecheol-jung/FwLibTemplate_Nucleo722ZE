// Firmware library2 utility
///////////////////////////////////////////////////////////////////////////////
// Base64 encoding/decoding
// https://opensource.apple.com/source/QuickTimeStreamingServer/QuickTimeStreamingServer-452/CommonUtilitiesLib/base64.c.auto.html
//
// CRC
// https://github.com/lammertb/libcrc
///////////////////////////////////////////////////////////////////////////////

// fl_util.h

#ifndef FL_UTIL_H
#define FL_UTIL_H

#include "fl_def.h"

#if FL_BYTE_ORDER == FL_BYTE_ORDER_BIG_ENDIAN
#define FL_SWAP_2BYTES(val) val
#define FL_SWAP_4BYTES(val) val
#define FL_SWAP_8BYTES(val) val
#else
#define FL_SWAP_2BYTES(val) ( (((val) >> 8) & 0x00FF) | (((val) << 8) & 0xFF00) )
#define FL_SWAP_4BYTES(val) ( (((val) >> 24) & 0x000000FF) | (((val) >>  8) & 0x0000FF00) |(((val) << 8) & 0x00FF0000) | (((val) << 24) & 0xFF000000))
#define FL_SWAP_8BYTES(val) ( (((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) | \
                                (((val) >> 24) & 0x0000000000FF0000) | (((val) >> 8) & 0x00000000FF000000) | \
                                (((val) << 8) & 0x000000FF00000000) | (((val) << 24) & 0x0000FF0000000000) | \
                                (((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000))
#endif

#define FL_BIT_FIELD_SET(field, value, mask, pos) ((field & ~mask) | ((value << pos) & mask))
#define FL_BIT_FIELD_GET(field, mask, pos)        ((field & mask) >> pos)

FL_BEGIN_DECLS

FL_DECLARE(uint16_t) fl_crc_16(const unsigned char* buf, size_t buf_size);
FL_DECLARE(int) fl_base64_encode_len(int len);
FL_DECLARE(int) fl_base64_encode(char* dst, const char* src, int src_len);
FL_DECLARE(int) fl_base64_decode_len(const char* src);
FL_DECLARE(int) fl_base64_decode(char* dst, const char* src);

FL_END_DECLS

#endif

