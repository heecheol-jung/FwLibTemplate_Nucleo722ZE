// Firmware library for digital I/O module
// fl_dio.h

#ifndef FL_DIO_H
#define FL_DIO_H

// Defines
#define FL_DIO_IN           (0)
#define FL_DIO_OUT          (1)

#define FL_DIO_ACTIVE_HIGH  (0)
#define FL_DIO_ACTIVE_LOW   (1)

#include "stm32f7xx_hal.h"
#include "fl_def.h"

// Structures.
FL_BEGIN_PACK1

typedef struct _fl_dio_port
{
  uint8_t       port_id;
  GPIO_TypeDef* port;
  uint16_t      port_pin;

  // IN or OUT.
  uint8_t       port_mode;

  // Active low or high.
  uint8_t       active_mode;
} fl_dio_port_t;

FL_END_PACK

// Functions.
FL_BEGIN_DECLS

FL_DECLARE(uint8_t) fl_dio_write(fl_dio_port_t *port_handle, fl_bool_t logical_on_off);
FL_DECLARE(uint8_t) fl_dio_read(fl_dio_port_t *port_handle, fl_bool_t *logical_on_off);

FL_END_DECLS

#endif
