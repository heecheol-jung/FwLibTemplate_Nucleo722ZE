// Firmware library for DS18B20
// Original project
// https://github.com/nimaltd/ds18b20
#ifndef FL_DS18B20_H
#define FL_DS18B20_H

#include "gpio.h"
#include "fl_def.h"
#include "fl_base_device.h"
#include "fl_stm32.h"
#include "fl_one_wire.h"

#define FL_DS18B20_ADDR_LEN     (8)
#define FL_DS18B20_MAX_SENSORS  (1)

#define FL_DS18B20_CONVERT_TIMEOUT_MS   5000

//###################################################################################
/* Every onewire chip has different ROM code, but all the same chips has same family code */
/* in case of DS18B20 this is 0x28 and this is first byte of ROM address */
#define FL_DS18B20_FAMILY_CODE          0x28
#define FL_DS18B20_CMD_ALARMSEARCH      0xEC

/* DS18B20 read temperature command */
#define FL_DS18B20_CMD_CONVERTTEMP      0x44  /* Convert temperature */
#define FL_DS18B20_DECIMAL_STEPS_12BIT  0.0625
#define FL_DS18B20_DECIMAL_STEPS_11BIT  0.125
#define FL_DS18B20_DECIMAL_STEPS_10BIT  0.25
#define FL_DS18B20_DECIMAL_STEPS_9BIT   0.5

#define FL_DS18B20_RESOLUTION_9BIT      9
#define FL_DS18B20_RESOLUTION_10BIT     10
#define FL_DS18B20_RESOLUTION_11BIT     11
#define FL_DS18B20_RESOLUTION_12BIT     12

/* Bits locations for resolution */
#define FL_DS18B20_RESOLUTION_R1         6
#define FL_DS18B20_RESOLUTION_R0         5

/* CRC enabled */
#ifdef FL_DS18B20_USE_CRC
#define FL_DS18B20_DATA_LEN              9
#else
#define FL_DS18B20_DATA_LEN              2
#endif

FL_BEGIN_DECLS

typedef struct _fl_ds18b20
{
  fl_base_device  common_data;
  uint8_t         addr[FL_DS18B20_ADDR_LEN];
  float           temperature;
  fl_bool_t       data_valid;
} fl_ds18b20;

typedef struct _fl_ds18b20_manager
{
  uint8_t     OneWireDevices;
  uint8_t     TempSensorCount;
  uint8_t     Ds18b20StartConvert;
  uint16_t    Ds18b20Timeout;
  fl_one_wire one_wire;
  fl_ds18b20  devices[FL_DS18B20_MAX_SENSORS];
} fl_ds18b20_manager;

FL_DECLARE(fl_bool_t)   fl_ds18b20_init(fl_ds18b20_manager* handle, GPIO_TypeDef* hport, uint16_t pin_num);
FL_DECLARE(fl_bool_t)   fl_ds18b20_manual_convert(fl_ds18b20_manager *handle);

FL_END_DECLS

#endif
