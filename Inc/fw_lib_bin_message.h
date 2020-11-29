// fw_lib_bin_message.h

#ifndef FW_LIB_BIN_MESSAGE_H
#define FW_LIB_BIN_MESSAGE_H

#include "fw_lib_message.h"

FW_LIB_BEGIN_DECLS

#define FW_LIB_BIN_MSG_STX                        (0x02)
#define FW_LIB_BIN_MSG_ETX                        (0x03)

// STX            Header                     Payload CRC ETX
//      Device_ID Length Msg_ID Flag1 Flag2 
//  1       4       1       1     1     1       250   2   1
//              (Max : 256)
// Payload max length = 256 -(Msg_ID+Flag1+Flag2)-(CRC+ETX)
//                    = 256 -(1+1+1)-(2+1)
//                    = 250
//
// STX Header Payload CRC ETX
//  1     8      250   2   1   = 262(264 : Multiple of 4)
#define FW_LIB_BIN_MSG_STX_LENGTH                 (1)
#define FW_LIB_BIN_MSG_HEADER_LENGTH              (8)
#define FW_LIB_BIN_MSG_MAX_PAYLOAD_LENGTH         (250)
#define FW_LIB_BIN_MSG_CRC_LENGTH                 (2)
#define FW_LIB_BIN_MSG_ETX_LENGTH                 (1)

// MAX_PAYLOAD(250) + CRC(2) + ETX(1) + Padding(2)= 255
#define FW_LIB_BIN_MSG_MAX_DATA_LENGTH            (255)
// STX(1) + HEADER(8) + CRC(2) + ETX(1)
#define FW_LIB_BIN_MSG_MIN_LENGTH                 (12)
// STX(1) + HEADER(8) + MAX_DATA(255)
#define FW_LIB_BIN_MSG_MAX_LENGTH                 (264)

// Device ID field of header.
#define FW_LIB_BIN_MSG_DEVICE_ID_LENGTH           (4)

#define FW_LIB_BIN_MSG_MIN_SEQUENCE               (0)
#define FW_LIB_BIN_MSG_MAX_SEQUENCE               (0xf)

// Header.Flag1 sequence number bit position and mask
#define FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS       (0x00)
#define FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK      (0x0F)

// Header.Flag1 return expected bit position and mask
#define FW_LIB_BIN_MSG_HDR_FLG1_RET_EXPECTED_POS  (0x04)
#define FW_LIB_BIN_MSG_HDR_FLG1_RET_EXPECTED_MASK (0x10)

// Header.Flag1 message type bit position and mask
#define FW_LIB_BIN_MSG_HDR_FLG1_MSG_TYPE_POS      (0x05)
#define FW_LIB_BIN_MSG_HDR_FLG1_MSG_TYPE_MASK     (0x60)

// Header.Flag1 Reserved bit position and mask
#define FW_LIB_BIN_MSG_HDR_FLG1_RESERVED_POS      (0x07)
#define FW_LIB_BIN_MSG_HDR_FLG1_RESERVED_MASK     (0x80)

// Header.Flag2 error bit position and mask
#define FW_LIB_BIN_MSG_HDR_FLG2_ERROR_POS         (0x00)
#define FW_LIB_BIN_MSG_HDR_FLG2_ERROR_MASK        (0x03)

// Header.Flag2 Reserved bit position and mask
#define FW_LIB_BIN_MSG_HDR_FLG2_RESERVED_POS      (0x02)
#define FW_LIB_BIN_MSG_HDR_FLG2_RESERVED_MASK     (0xFC)

FW_LIB_BEGIN_PACK1

typedef struct _fw_lib_bin_msg_header
{
  // Unique device ID(for RS-422, RS-485).
  uint32_t  device_id;

  // Message length.
  uint8_t   length;

  // Message(function) ID.
  uint8_t   message_id;

  //uint8_t   sequence_number : 4,
  //          return_expected : 1,
  //          message_type : 2,     // fw_lib_msg_type_t
  //          flag1_reserved1 : 1;
  uint8_t   flag1;

  //uint8_t   error : 2,
  //          flag2_reserved : 6;
  uint8_t   flag2;
} fw_lib_bin_msg_header_t;

// Full message(including STX and ETX).
typedef struct _fw_lib_bin_msg_full
{
  uint8_t                 stx;
  fw_lib_bin_msg_header_t header;
  uint8_t                 data[FW_LIB_BIN_MSG_MAX_DATA_LENGTH];
} fw_lib_bin_msg_full_t;

// Read hardware version response.
typedef struct _fw_bin_msg_read_hw_ver_resp
{
  fw_lib_bin_msg_header_t header;
  uint8_t                 major;
  uint8_t                 minor;
  uint8_t                 revision;
} fw_bin_msg_read_hw_ver_resp_t;

// Read firmware version response.
typedef struct _fw_bin_msg_read_fw_ver_resp
{
  fw_lib_bin_msg_header_t header;
  uint8_t                 major;
  uint8_t                 minor;
  uint8_t                 revision;
} fw_bin_msg_read_fw_ver_resp_t;

// Read GPIO port command.
typedef struct _fw_bin_msg_read_gpio_cmd
{
  fw_lib_bin_msg_header_t header;
  uint8_t                 port_number;
} fw_bin_msg_read_gpio_cmd_t;

// Read GPIO port response.
typedef struct _fw_bin_msg_read_gpio_resp
{
  fw_lib_bin_msg_header_t header;
  uint8_t                 port_number;
  uint8_t                 port_value;
} fw_bin_msg_read_gpio_resp_t;

// Write GPIO port command.
typedef struct _fw_bin_msg_write_gpio_cmd
{
  fw_lib_bin_msg_header_t header;
  uint8_t                 port_number;
  uint8_t                 port_value;
} fw_bin_msg_write_gpio_cmd_t;

// Button event.
typedef struct _fw_bin_msg_button_evt
{
  fw_lib_bin_msg_header_t header;
  uint8_t                 button_number;
  uint8_t                 button_status;  
} fw_bin_msg_button_evt_t;

// Read DHT22 temperature/humidity command.
typedef struct _fw_bin_msg_read_dht22_cmd
{
  fw_lib_bin_msg_header_t header;
  uint8_t                 sensor_number;
} fw_bin_msg_read_dht22_cmd_t;

// Read DHT22 temperature/humidity response.
typedef struct _fw_bin_msg_read_dht22_resp
{
  fw_lib_bin_msg_header_t header;
  uint8_t                 sensor_number;
  uint16_t                sensor_value;
} fw_bin_msg_read_dht22_resp_t;


FW_LIB_END_PACK

FW_LIB_DECLARE(uint8_t) fw_lib_bin_msg_build_command(const uint32_t device_id, const uint8_t message_id, const uint8_t sequnece_num, const uint8_t return_expected, uint8_t* packet_buf);
FW_LIB_DECLARE(uint8_t) fw_lib_bin_msg_build_response(const uint32_t device_id, const uint8_t message_id, const uint8_t sequnece_num, const uint8_t return_expected, uint8_t error, uint8_t* packet_buf);
FW_LIB_DECLARE(uint8_t) fw_lib_bin_msg_build_event(const uint32_t device_id, const uint8_t message_id, const uint8_t sequnece_num, uint8_t* packet_buf);

FW_LIB_END_DECLS

#endif
