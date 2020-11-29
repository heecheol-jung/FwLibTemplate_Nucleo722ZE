// fw_lib_base_device.h

#ifndef FW_LIB_BASE_DEVICE_H
#define FW_LIB_BASE_DEVICE_H

#include "fw_lib_def.h"

#define FW_LIB_DEVICE_ID_UNKNOWN    (0)
//#define FW_LIB_DEVICE_ID_ALL        (0xffffffff)

#define FW_LIB_DEVICE_TYPE_UNKNOWN  (0)
#define FW_LIB_DEVICE_TYPE_IO       (1)
#define FW_LIB_DEVICE_TYPE_SENSOR   (2)

typedef void(*fw_lib_gpio_write_t)(const void* gpio_handle, fw_lib_bool_t on_off);
typedef fw_lib_bool_t(*fw_lib_gpio_read_t)(const void* gpio_handle);
typedef void(*fw_lib_delay_us_t)(volatile uint32_t microseconds);

typedef struct _fw_lib_base_device
{
  uint32_t  device_id;
  uint8_t   device_type;
} fw_lib_base_device;

#endif
