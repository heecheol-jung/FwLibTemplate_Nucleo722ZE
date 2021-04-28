// Firmware library binary message
// fl_bin_message.h

#ifndef FL_BIN_MESSAGE_H
#define FL_BIN_MESSAGE_H

#include "fl_message_def.h"

FL_BEGIN_DECLS

#define FL_BIN_MSG_STX                        (0x02)
#define FL_BIN_MSG_ETX                        (0x03)

// STX            Header                     Payload CRC ETX
//      Device_ID Length Msg_ID Flag1 Flag2 
//  1       4       1       1     1     1       32   2   1
//     |-----------------------------------|
//                      8
#define FL_BIN_MSG_STX_LENGTH                 (1)
#define FL_BIN_MSG_HEADER_LENGTH              (8)
#define FL_BIN_MSG_MAX_PAYLOAD_LENGTH         (32)
#define FL_BIN_MSG_CRC_LENGTH                 (2)
#define FL_BIN_MSG_ETX_LENGTH                 (1)

// MAX_PAYLOAD(32) + CRC(2) + ETX(1) + Padding(1)= 36
#define FL_BIN_MSG_MAX_DATA_LENGTH            (36)
// STX(1) + HEADER(8) + CRC(2) + ETX(1)
#define FL_BIN_MSG_MIN_LENGTH                 (12)
// STX(1) + HEADER(8) + MAX_PAYLOAD(32) + CRC(2) + ETX(1)
#define FL_BIN_MSG_MAX_LENGTH                 (44)

// Device ID field of header.
#define FL_BIN_MSG_DEVICE_ID_LENGTH           (4)

#define FL_BIN_MSG_MIN_SEQUENCE               (0)
#define FL_BIN_MSG_MAX_SEQUENCE               (0xf)

FL_BEGIN_PACK1
///////////////////////////////////////////////////////////////////////////////
// structs for binary messages
///////////////////////////////////////////////////////////////////////////////

// Binary message header flag1
typedef struct _fl_bin_msg_flag1
{
  uint8_t   reserved : 1;         // Lower bit
  uint8_t   message_type : 2;
  uint8_t   return_expected : 1;
  uint8_t   sequence_num : 4;     // Higher bit
} fl_bin_msg_flag1_t;

// Binary message header flag2
typedef struct _fl_bin_msg_flag2
{
  uint8_t   reserved : 6;
  uint8_t   error : 2;
} fl_bin_msg_flag2_t;

// Binary message header
typedef struct _fl_bin_msg_header
{
  // Unique device ID(for RS-422, RS-485).
  uint32_t              device_id;

  // Message length.
  uint8_t               length;

  // Message(function) ID.
  uint8_t               message_id;

  // Flag1
  fl_bin_msg_flag1_t   flag1;

  // Flag2
  fl_bin_msg_flag2_t   flag2;
} fl_bin_msg_header_t;

// Full message(including STX and ETX).
typedef struct _fl_bin_msg_full
{
  uint8_t               stx;
  fl_bin_msg_header_t  header;
  fl_hw_ver_t          payload;  // fl_hw_ver_t is the maximimum payload.
  uint16_t              crc;
  uint8_t               etx;
} fl_bin_msg_full_t;

FL_END_PACK

FL_DECLARE(uint8_t) fl_bin_msg_build_command(uint8_t* packet_buf, uint16_t packet_buf_len);
FL_DECLARE(uint8_t) fl_bin_msg_build_response(uint8_t* packet_buf, uint16_t packet_buf_len);
FL_DECLARE(uint8_t) fl_bin_msg_build_event(uint8_t* packet_buf, uint16_t packet_buf_len);

FL_END_DECLS

#endif

