// Firware library message
// fl_message_def.h

#ifndef FL_MESSAGE_DEF_H
#define FL_MESSAGE_DEF_H

#include "fl_def.h"

FL_BEGIN_DECLS

///////////////////////////////////////////////////////////////////////////////
// Message ID's
///////////////////////////////////////////////////////////////////////////////
// Message IDs
#define FL_MSG_ID_BASE                      (0)
#define FL_MSG_ID_UNKNOWN                   (FL_MSG_ID_BASE + 0)

// Read hardware version.
#define FL_MSG_ID_READ_HW_VERSION           (FL_MSG_ID_BASE + 1)

// Read firmware version.
#define FL_MSG_ID_READ_FW_VERSION           (FL_MSG_ID_BASE + 2)

// Read a value from a GPIO input pin.
#define FL_MSG_ID_READ_GPIO                 (FL_MSG_ID_BASE + 3)

// Write a value to a GPIO output pin.
#define FL_MSG_ID_WRITE_GPIO                (FL_MSG_ID_BASE + 4)

// Button event.
#define FL_MSG_ID_BUTTON_EVENT              (FL_MSG_ID_BASE + 5)

// Read temperature.
#define FL_MSG_ID_READ_TEMPERATURE          (FL_MSG_ID_BASE + 6)

// Read humidity.
#define FL_MSG_ID_READ_HUMIDITY             (FL_MSG_ID_BASE + 7)

// Read temperature and humidity.
#define FL_MSG_ID_READ_TEMP_AND_HUM         (FL_MSG_ID_BASE + 8)

// Set boot mode. 
#define FL_MSG_ID_BOOT_MODE                 (FL_MSG_ID_BASE + 9)

// Reset a target device.
#define FL_MSG_ID_RESET                     (FL_MSG_ID_BASE + 10)

///////////////////////////////////////////////////////////////////////////////
// Defines for general messages.
///////////////////////////////////////////////////////////////////////////////
// Maximum length of argument string.
#define FL_MSG_MAX_STRING_LEN               (32)

// Device IDs
#define FL_DEVICE_ID_UNKNOWN                (0)

#define FL_DEVICE_ID_ALL                    (0xFFFFFFFF)  // Device broadcasting.

#define FL_BUTTON_RELEASED                  (0)

#define FL_BUTTON_PRESSED                   (1)

// Message type.
#define FL_MSG_TYPE_COMMAND                 (0)
#define FL_MSG_TYPE_RESPONSE                (1)
#define FL_MSG_TYPE_EVENT                   (2)
#define FL_MSG_TYPE_UNKNOWN                 (0XFF)

// Boot mode : Application
#define FL_BMODE_APP                        (0)
// Boot mode : Bootloader
#define FL_BMODE_BOOTLOADER                 (1)

#define FL_VER_STR_MAX_LEN                  (32)

#define FL_TXT_MSG_ID_MIN_CHAR              ('A')
#define FL_TXT_MSG_ID_MAX_CHAR              ('Z')
#define FL_TXT_DEVICE_ID_MIN_CHAR           ('0')
#define FL_TXT_DEVICE_ID_MAX_CHAR           ('9')
// The last character for a text message.
#define FL_TXT_MSG_TAIL                     ('\n')
// Delimiter for a message id and device id
#define FL_TXT_MSG_ID_DEVICE_ID_DELIMITER   (' ')
// Delimiter for arguments.
#define FL_TXT_MSG_ARG_DELIMITER            (',')

FL_BEGIN_PACK1

///////////////////////////////////////////////////////////////////////////////
// structs for messages.
///////////////////////////////////////////////////////////////////////////////
typedef struct _fl_hw_ver
{
  char        version[FL_VER_STR_MAX_LEN];
} fl_hw_ver_t;

typedef struct _fl_fw_ver
{
  char        version[FL_VER_STR_MAX_LEN];
} fl_fw_ver_t;

typedef struct _fl_gpi_port
{
  uint8_t     port_num;
} fl_gpi_port_t;

typedef struct _fl_gpi_port_value
{
  uint8_t     port_num;
  uint8_t     port_value;
} fl_gpi_port_value_t;

typedef struct _fl_gpo_port_value
{
  uint8_t     port_num;
  uint8_t     port_value;
} fl_gpo_port_value_t;

typedef struct _fl_btn_status
{
  uint8_t     button_num;
  uint8_t     button_value;
} fl_btn_status_t;

typedef struct _fl_sensor
{
  uint8_t     sensor_num;
} fl_sensor_t;

typedef struct _fl_temp_sensor_read
{
  uint8_t     sensor_num;
  double      temperature;
} fl_temp_sensor_read_t;

typedef struct _fl_hum_sensor_read
{
  uint8_t     sensor_num;
  double      humidity;
} fl_hum_sensor_read_t;

typedef struct _fl_temp_hum_sensor_read
{
  uint8_t     sensor_num;
  double      temperature;
  double      humidity;
} fl_temp_hum_sensor_read_t;

typedef struct _fl_boot_mode
{
  uint8_t     boot_mode;  // FL_BMODE_APP, FL_BMODE_BOOTLOADER
} fl_boot_mode_t;

FL_END_PACK

typedef void(*fl_msg_cb_on_parsed_t)(const void* parser_handle, void* context);

// Parser debugging purpose(parsing time check, ...)
typedef void(*fl_msg_dbg_cb_on_parse_started_t)(const void* parser_handle);
typedef void(*fl_msg_dbg_cb_on_parse_ended_t)(const void* parser_handle);

FL_END_DECLS

#endif

