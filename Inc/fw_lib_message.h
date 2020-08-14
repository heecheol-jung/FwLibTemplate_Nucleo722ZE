// fw_lib_message.h

#ifndef FW_LIB_MESSAGE_H
#define FW_LIB_MESSAGE_H

#include "fw_lib_def.h"

FW_LIB_BEGIN_DECLS

// Maximum length of argument string.
#define FW_LIB_MSG_MAX_STRING_LEN         (32)

// Message IDs
#define FW_LIB_MSG_ID_UNKNOWN             (0)

// Read hardware version.
#define FW_LIB_MSG_ID_READ_HW_VERSION     (1)

// Read firmware version.
#define FW_LIB_MSG_ID_READ_FW_VERSION     (2)

// Read a value from a GPIO input pin.
#define FW_LIB_MSG_ID_READ_GPIO           (3)

// Write a value to a GPIO output pin.
#define FW_LIB_MSG_ID_WRITE_GPIO          (4)

// Button event.
#define FW_LIB_MSG_ID_BUTTON_EVENT        (5)

// Device IDs
#define FW_LIB_DEVICE_ID_UNKNOWN          (0)

#define FW_LIB_DEVICE_ID_ALL              (0xFFFFFFFF)  // Device broadcasting.

#define FW_LIB_BUTTON_RELEASED            (0)

#define FW_LIB_BUTTON_PRESSED             (1)

// Message type.
#define FW_LIB_MSG_TYPE_COMMAND           (0)
#define FW_LIB_MSG_TYPE_RESPONSE          (1)
#define FW_LIB_MSG_TYPE_EVENT             (2)
#define FW_LIB_MSG_TYPE_UNKNOWN           (0XFF)

// Argument type.
#define FW_LIB_ARG_TYPE_UNKNOWN           (0)
#define FW_LIB_ARG_TYPE_INT8              (1)
#define FW_LIB_ARG_TYPE_UINT8             (2)
#define FW_LIB_ARG_TYPE_INT16             (3)
#define FW_LIB_ARG_TYPE_UINT16            (4)
#define FW_LIB_ARG_TYPE_INT32             (5)
#define FW_LIB_ARG_TYPE_UINT32            (6)
#define FW_LIB_ARG_TYPE_INT64             (7)
#define FW_LIB_ARG_TYPE_UINT64            (8)
#define FW_LIB_ARG_TYPE_STRING            (9)
#define FW_LIB_ARG_TYPE_CUSTOM            (0xFF)

FW_LIB_BEGIN_PACK1

// Message argument structure
typedef struct _fw_lib_msg_arg
{
  uint8_t     type;
  union
  {
    int8_t    int8_value;
    uint8_t   uint8_value;
    int16_t   int16_value;
    uint16_t  uint16_value;
    int32_t   int32_value;
    uint32_t  uint32_value;
    int64_t   int64_value;
    uint64_t  uint64_value;
    char      string_value[FW_LIB_MSG_MAX_STRING_LEN];
  } value;
} fw_lib_msg_arg_t;

FW_LIB_END_PACK

typedef void(*fw_lib_msg_cb_on_parsed_t)(const void* parser_handle, void* context);

FW_LIB_END_DECLS

#endif
