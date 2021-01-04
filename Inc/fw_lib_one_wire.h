// https://github.com/nimaltd/ds18b20/blob/master/onewire.h

#ifndef FW_LIB_ONE_WIRE_H
#define FW_LIB_ONE_WIRE_H

#include "gpio.h"
#include "fw_lib_def.h"
#include "fw_lib_base_device.h"
#include "fw_lib_stm32.h"

/* OneWire commands */
#define FW_LIB_OW_CMD_RSCRATCHPAD     0xBE
#define FW_LIB_OW_CMD_WSCRATCHPAD     0x4E
#define FW_LIB_OW_CMD_CPYSCRATCHPAD   0x48
#define FW_LIB_OW_CMD_RECEEPROM       0xB8
#define FW_LIB_OW_CMD_RPWRSUPPLY      0xB4
#define FW_LIB_OW_CMD_SEARCHROM       0xF0
#define FW_LIB_OW_CMD_READROM         0x33
#define FW_LIB_OW_CMD_MATCHROM        0x55
#define FW_LIB_OW_CMD_SKIPROM         0xCC

FW_LIB_BEGIN_DECLS

typedef struct _fw_lib_one_wire
{
  GPIO_TypeDef* gpio_handle;
  uint16_t gpio_pin;
  uint8_t last_discrepancy;
  uint8_t last_family_discrepancy;
  uint8_t last_device_flag;
  uint8_t rom_no[8];

  fw_lib_delay_us_t   cb_delay_us;
} fw_lib_one_wire;

FW_LIB_DECLARE(void)    fw_lib_ow_init(fw_lib_one_wire *handle, GPIO_TypeDef* gpio_handle, uint16_t gpio_pin);
FW_LIB_DECLARE(void)    fw_lib_ow_low(fw_lib_one_wire *handle);
FW_LIB_DECLARE(void)    fw_lib_ow_high(fw_lib_one_wire *handle);
FW_LIB_DECLARE(void)    fw_lib_ow_input(fw_lib_one_wire *handle);
FW_LIB_DECLARE(void)    fw_lib_ow_output(fw_lib_one_wire *handle);
FW_LIB_DECLARE(uint8_t) fw_lib_ow_reset(fw_lib_one_wire *handle);
FW_LIB_DECLARE(uint8_t) fw_lib_ow_read_byte(fw_lib_one_wire *handle);
FW_LIB_DECLARE(uint8_t) fw_lib_ow_read_bit(fw_lib_one_wire *handle);
FW_LIB_DECLARE(void)    fw_lib_ow_write_byte(fw_lib_one_wire *handle, uint8_t val);
FW_LIB_DECLARE(void)    fw_lib_ow_write_bit(fw_lib_one_wire *handle, uint8_t bit);
FW_LIB_DECLARE(uint8_t) fw_lib_ow_search(fw_lib_one_wire *handle, uint8_t command);
FW_LIB_DECLARE(void)    fw_lib_ow_reset_search(fw_lib_one_wire *handle);
FW_LIB_DECLARE(uint8_t) fw_lib_ow_first(fw_lib_one_wire *handle);
FW_LIB_DECLARE(uint8_t) fw_lib_ow_next(fw_lib_one_wire *handle);
FW_LIB_DECLARE(void)    fw_lib_ow_get_full_rom(fw_lib_one_wire *handle, uint8_t* first_idx);
FW_LIB_DECLARE(void)    fw_lib_ow_select(fw_lib_one_wire *handle, uint8_t* addr);
FW_LIB_DECLARE(void)    fw_lib_ow_select_with_pointer(fw_lib_one_wire *handle, uint8_t* rom);
FW_LIB_DECLARE(uint8_t) fw_lib_ow_crc8(uint8_t *addr, uint8_t len);

FW_LIB_END_DECLS

#endif
