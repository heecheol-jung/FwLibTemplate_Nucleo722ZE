// Firmware library text message
// fl_txt_message.h

#ifndef FL_TXT_MESSAGE_H
#define FL_TXT_MESSAGE_H

#include "fl_message_def.h"

FL_BEGIN_DECLS

#define FL_TXT_MSG_MAX_LENGTH           (64)

#define FL_TXT_MSG_ID_MAX_LEN           (5)

#define FL_TXT_MSG_DEVICE_ID_MAX_LEN    (2)

#define FL_TXT_MSG_ERROR_MAX_LEN        (3)

#define FL_TXT_MSG_MAX_ARG_COUNT        (3)

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

#define FL_TXT_RHVER_STR                ("RHVER")   // Read hardware version.
#define FL_TXT_RFVER_STR                ("RFVER")   // Read firmware version.
#define FL_TXT_RGPIO_STR                ("RGPIO")   // Read a value from a GPIO input pin.
#define FL_TXT_WGPIO_STR                ("WGPIO")   // Write a value to a GPIO output pin.
#define FL_TXT_EBTN_STR                 ("EBTN")    // Button event.
#define FL_TXT_RTEMP_STR                ("RTEMP")   // Read temperature.
#define FL_TXT_RHUM_STR                 ("RHUM")    // Read humidity.
#define FL_TXT_RTAH_STR                 ("RTAH")    // Read temperature and humidity.
#define FL_TXT_BMODE_STR                ("BMODE")   // Set boot mode.
#define FL_TXT_RESET_STR                ("RESET")   // Reset a target device.

FL_BEGIN_PACK1

typedef struct _fl_txt_msg
{
  // Unique device ID(for RS-422, RS-485).
  uint32_t          device_id;

  // Message ID.
  uint8_t           msg_id;

  // Error code.
  uint8_t           error;

  // It is for maximum message buffer.
  fl_fw_ver_t       payload;
} fl_txt_msg_t;

FL_END_PACK

FL_DECLARE(uint8_t) fl_txt_msg_build_command(const uint32_t device_id, const uint8_t message_id, void* arg_buf, uint32_t arg_buf_len, uint8_t* packet_buf, uint32_t packet_buf_len);
FL_DECLARE(uint8_t) fl_txt_msg_build_response(const uint32_t device_id, const uint8_t message_id, uint8_t error, void* arg_buf, uint32_t arg_buf_len, uint8_t* packet_buf, uint32_t packet_buf_len);
FL_DECLARE(uint8_t) fl_txt_msg_build_event(const uint32_t device_id, const uint8_t message_id, void* arg_buf, uint32_t arg_buf_len, uint8_t* packet_buf, uint32_t packet_buf_len);
FL_DECLARE(char*) fl_txt_msg_get_message_name(const uint8_t message_id);

FL_END_DECLS

#endif
