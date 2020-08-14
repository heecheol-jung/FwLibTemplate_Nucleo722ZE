// fw_lib_util.h

#ifndef FW_LIB_UTIL_H
#define FW_LIB_UTIL_H

#include "fw_lib_def.h"

#if FW_LIB_BYTE_ORDER == FW_LIB_BYTE_ORDER_BIG_ENDIAN
#define FW_LIB_SWAP_2BYTES(val) val
#define FW_LIB_SWAP_4BYTES(val) val
#define FW_LIB_SWAP_8BYTES(val) val
#else
#define FW_LIB_SWAP_2BYTES(val) ( (((val) >> 8) & 0x00FF) | (((val) << 8) & 0xFF00) )
#define FW_LIB_SWAP_4BYTES(val) ( (((val) >> 24) & 0x000000FF) | (((val) >>  8) & 0x0000FF00) |(((val) << 8) & 0x00FF0000) | (((val) << 24) & 0xFF000000))
#define FW_LIB_SWAP_8BYTES(val) ( (((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) | \
                                  (((val) >> 24) & 0x0000000000FF0000) | (((val) >> 8) & 0x00000000FF000000) | \
                                  (((val) << 8) & 0x000000FF00000000) | (((val) << 24) & 0x0000FF0000000000) | \
                                  (((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000))
#endif

#define FW_LIB_BIT_FIELD_SET(field, value, mask, pos) ((field & ~mask) | ((value << pos) & mask))
#define FW_LIB_BIT_FIELD_GET(field, mask, pos)        ((field & mask) >> pos)

FW_LIB_BEGIN_DECLS

FW_LIB_DECLARE(uint16_t) fw_lib_crc_16(const unsigned char* buf, size_t buf_size);

FW_LIB_END_DECLS

#endif
