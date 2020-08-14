// fw_lib_txt_message.h

#ifndef FW_LIB_TXT_MESSAGE_H
#define FW_LIB_TXT_MESSAGE_H

#include "fw_lib_message.h"

FW_LIB_BEGIN_DECLS

#define FW_LIB_TXT_MSG_MAX_LENGTH           (64)

#define FW_LIB_TXT_MSG_MAX_ARG_COUNT        (3)

// The last character for a message.
#define FW_LIB_TXT_MSG_TAIL                 ('\n')

// Text message examples
// RHVER 1\n
//   |   |----> device id
//   |-------> command(read heardware version)
//
// RHVER 1,1,0.0.1\n
//   |   | |   |----> version string
//   |   | |--------> result(ok, fail)
//   |   |----------> device id
//   |--------------> response
//
// WGPIO 1,1,1\n
//    |  | | |---> GPIO output value
//    |  | |-----> GPIO id
//    |----------> devicd id

#define FW_LIB_TXT_RHVER_STR                ("RHVER")
#define FW_LIB_TXT_RFVER_STR                ("RFVER")
#define FW_LIB_TXT_RGPIO_STR                ("RGPIO")
#define FW_LIB_TXT_WGPIO_STR                ("WGPIO")
#define FW_LIB_TXT_EBTN_STR                 ("EBTN")

FW_LIB_BEGIN_PACK1

typedef struct _fw_lib_txt_msg
{
  // Unique device ID(for RS-422, RS-485).
  uint32_t          device_id;
  uint8_t           msg_id;
  fw_lib_msg_arg_t  args[FW_LIB_TXT_MSG_MAX_ARG_COUNT];
  uint8_t           arg_count;
} fw_lib_txt_msg_t;

FW_LIB_END_PACK

FW_LIB_DECLARE(uint8_t) fw_lib_txt_msg_build_command(const uint32_t device_id, const uint8_t message_id, fw_lib_msg_arg_t* args, uint8_t* packet_buf);
FW_LIB_DECLARE(uint8_t) fw_lib_txt_msg_build_response(const uint32_t device_id, const uint8_t message_id, fw_lib_msg_arg_t* args, uint8_t error, uint8_t* packet_buf);
FW_LIB_DECLARE(uint8_t) fw_lib_txt_msg_build_event(const uint32_t device_id, const uint8_t message_id, fw_lib_msg_arg_t* args, uint8_t* packet_buf);
FW_LIB_DECLARE(char*)   fw_lib_txt_msg_get_message_name(const uint8_t message_id);

FW_LIB_END_DECLS

#endif
