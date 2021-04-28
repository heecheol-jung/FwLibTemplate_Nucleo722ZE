// Firmware library for One wire
// https://github.com/nimaltd/ds18b20/blob/master/onewire.h

#ifndef FL_ONE_WIRE_H
#define FL_ONE_WIRE_H

#include "gpio.h"
#include "fl_def.h"
#include "fl_base_device.h"
#include "fl_stm32.h"

/* OneWire commands */
#define FL_OW_CMD_RSCRATCHPAD     0xBE
#define FL_OW_CMD_WSCRATCHPAD     0x4E
#define FL_OW_CMD_CPYSCRATCHPAD   0x48
#define FL_OW_CMD_RECEEPROM       0xB8
#define FL_OW_CMD_RPWRSUPPLY      0xB4
#define FL_OW_CMD_SEARCHROM       0xF0
#define FL_OW_CMD_READROM         0x33
#define FL_OW_CMD_MATCHROM        0x55
#define FL_OW_CMD_SKIPROM         0xCC

FL_BEGIN_DECLS

typedef struct _fl_one_wire
{
  GPIO_TypeDef* gpio_handle;
  uint16_t gpio_pin;
  uint8_t last_discrepancy;
  uint8_t last_family_discrepancy;
  uint8_t last_device_flag;
  uint8_t rom_no[8];

  fl_delay_us_t   cb_delay_us;
} fl_one_wire;

FL_DECLARE(void)    fl_ow_init(fl_one_wire *handle, GPIO_TypeDef* gpio_handle, uint16_t gpio_pin);
FL_DECLARE(void)    fl_ow_low(fl_one_wire *handle);
FL_DECLARE(void)    fl_ow_high(fl_one_wire *handle);
FL_DECLARE(void)    fl_ow_input(fl_one_wire *handle);
FL_DECLARE(void)    fl_ow_output(fl_one_wire *handle);
FL_DECLARE(uint8_t) fl_ow_reset(fl_one_wire *handle);
FL_DECLARE(uint8_t) fl_ow_read_byte(fl_one_wire *handle);
FL_DECLARE(uint8_t) fl_ow_read_bit(fl_one_wire *handle);
FL_DECLARE(void)    fl_ow_write_byte(fl_one_wire *handle, uint8_t val);
FL_DECLARE(void)    fl_ow_write_bit(fl_one_wire *handle, uint8_t bit);
FL_DECLARE(uint8_t) fl_ow_search(fl_one_wire *handle, uint8_t command);
FL_DECLARE(void)    fl_ow_reset_search(fl_one_wire *handle);
FL_DECLARE(uint8_t) fl_ow_first(fl_one_wire *handle);
FL_DECLARE(uint8_t) fl_ow_next(fl_one_wire *handle);
FL_DECLARE(void)    fl_ow_get_full_rom(fl_one_wire *handle, uint8_t* first_idx);
FL_DECLARE(void)    fl_ow_select(fl_one_wire *handle, uint8_t* addr);
FL_DECLARE(void)    fl_ow_select_with_pointer(fl_one_wire *handle, uint8_t* rom);
FL_DECLARE(uint8_t) fl_ow_crc8(uint8_t *addr, uint8_t len);

FL_END_DECLS

#endif
