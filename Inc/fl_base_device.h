// Firmware library for base device
// fw_lib_base_device.h

#ifndef FL_BASE_DEVICE_H
#define FL_BASE_DEVICE_H

#include "fl_def.h"

#define FL_DEVICE_ID_UNKNOWN    (0)
//#define FL_DEVICE_ID_ALL        (0xffffffff)

#define FL_DEVICE_TYPE_UNKNOWN  (0)
#define FL_DEVICE_TYPE_IO       (1)
#define FL_DEVICE_TYPE_SENSOR   (2)

typedef void(*fl_gpio_write_t)(const void* gpio_handle, fl_bool_t on_off);
typedef fl_bool_t(*fl_gpio_read_t)(const void* gpio_handle);
typedef void(*fl_delay_us_t)(volatile uint32_t microseconds);

typedef struct _fl_base_device
{
  uint32_t  device_id;
  uint8_t   device_type;
} fl_base_device;

#endif
