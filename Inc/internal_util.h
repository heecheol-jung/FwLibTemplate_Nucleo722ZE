#ifndef INTERNAL_UTIL_H
#define INTERNAL_UTIL_H

#include "fl_def.h"

fl_bool_t is_msg_id_char(uint8_t data);
fl_bool_t is_device_id_char(uint8_t data);
uint8_t get_device_id(uint8_t* buf, uint16_t buf_size);
fl_bool_t is_tail(uint8_t data);

#endif

