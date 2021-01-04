// Original project
// https://github.com/nimaltd/ds18b20
#ifndef FW_LIB_DS18B20_H
#define FW_LIB_DS18B20_H

#include "gpio.h"
#include "fw_lib_def.h"
#include "fw_lib_base_device.h"
#include "fw_lib_stm32.h"
#include "fw_lib_one_wire.h"

#define FW_LIB_DS18B20_ADDR_LEN     (8)
#define FW_LIB_DS18B20_MAX_SENSORS  (1)

#define FW_LIB_DS18B20_CONVERT_TIMEOUT_MS   5000

//###################################################################################
/* Every onewire chip has different ROM code, but all the same chips has same family code */
/* in case of DS18B20 this is 0x28 and this is first byte of ROM address */
#define FW_LIB_DS18B20_FAMILY_CODE          0x28
#define FW_LIB_DS18B20_CMD_ALARMSEARCH      0xEC

/* DS18B20 read temperature command */
#define FW_LIB_DS18B20_CMD_CONVERTTEMP      0x44  /* Convert temperature */
#define FW_LIB_DS18B20_DECIMAL_STEPS_12BIT  0.0625
#define FW_LIB_DS18B20_DECIMAL_STEPS_11BIT  0.125
#define FW_LIB_DS18B20_DECIMAL_STEPS_10BIT  0.25
#define FW_LIB_DS18B20_DECIMAL_STEPS_9BIT   0.5

#define FW_LIB_DS18B20_RESOLUTION_9BIT      9
#define FW_LIB_DS18B20_RESOLUTION_10BIT     10
#define FW_LIB_DS18B20_RESOLUTION_11BIT     11
#define FW_LIB_DS18B20_RESOLUTION_12BIT     12

/* Bits locations for resolution */
#define FW_LIB_DS18B20_RESOLUTION_R1         6
#define FW_LIB_DS18B20_RESOLUTION_R0         5

/* CRC enabled */
#ifdef FW_LIB_DS18B20_USE_CRC
#define FW_LIB_DS18B20_DATA_LEN              9
#else
#define FW_LIB_DS18B20_DATA_LEN              2
#endif

FW_LIB_BEGIN_DECLS

typedef struct _fw_lib_ds18b20
{
  fw_lib_base_device  common_data;
  uint8_t             addr[FW_LIB_DS18B20_ADDR_LEN];
  float               temperature;
  fw_lib_bool_t       data_valid;
} fw_lib_ds18b20;

typedef struct _fw_lib_ds18b20_manager
{
  uint8_t         OneWireDevices;
  uint8_t         TempSensorCount;
  uint8_t         Ds18b20StartConvert;
  uint16_t        Ds18b20Timeout;
  fw_lib_one_wire one_wire;
  fw_lib_ds18b20  devices[FW_LIB_DS18B20_MAX_SENSORS];
} fw_lib_ds18b20_manager;

FW_LIB_DECLARE(fw_lib_bool_t)   fw_lib_ds18b20_init(fw_lib_ds18b20_manager* handle, GPIO_TypeDef* hport, uint16_t pin_num);
FW_LIB_DECLARE(fw_lib_bool_t)   fw_lib_ds18b20_manual_convert(fw_lib_ds18b20_manager *handle);

FW_LIB_END_DECLS

#endif
