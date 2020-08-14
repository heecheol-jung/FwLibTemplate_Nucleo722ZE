// Digital I/O module
// fw_lib_dio.h

#ifndef FW_LIB_DIO_H
#define FW_LIB_DIO_H

// Defines
#define FW_LIB_DIO_IN           (0)
#define FW_LIB_DIO_OUT          (1)

#define FW_LIB_DIO_ACTIVE_HIGH  (0)
#define FW_LIB_DIO_ACTIVE_LOW   (1)

// Structures.
FW_LIB_BEGIN_PACK1

typedef struct _fw_lib_dio_port
{
  uint8_t       port_id;
  GPIO_TypeDef* port;
  uint16_t      port_pin;

  // IN or OUT.
  uint8_t       port_mode;

  // Active low or high.
  uint8_t       active_mode;
} fw_lib_dio_port_t;

FW_LIB_END_PACK

// Functions.
FW_LIB_BEGIN_DECLS

FW_LIB_DECLARE(uint8_t) fw_lib_dio_write(fw_lib_dio_port_t *port_handle, fw_lib_bool_t logical_on_off);
FW_LIB_DECLARE(uint8_t) fw_lib_dio_read(fw_lib_dio_port_t *port_handle, fw_lib_bool_t *logical_on_off);

FW_LIB_END_DECLS

#endif
