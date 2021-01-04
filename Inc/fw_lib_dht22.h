// fw_lib_dht22.h

#ifndef FW_LIB_DHT22_H
#define FW_LIB_DHT22_H

// https://github.com/LonelyWolf/stm32/blob/master/am2302/dht22.h

#include "gpio.h"
#include "fw_lib_def.h"
#include "fw_lib_base_device.h"
#include "fw_lib_stm32.h"

#define FW_LIB_DHT22_MAX_BIT_LEN  (40)

FW_LIB_BEGIN_DECLS

// Read interval : 2 seconds
// https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/#:~:text=The%20DHT22%20sensor%20has%20a%20better%20resolution%20and,However%2C%20you%20can%20request%20sensor%20readings%20every%20second.
typedef struct _fw_lib_dht22
{
  fw_lib_base_device  common_data;
//  GPIO_TypeDef*       hport;
//  uint16_t            pin_num;
  uint8_t             hmsb;
  uint8_t             hlsb;
  uint8_t             tmsb;
  uint8_t             tlsb;
  uint8_t             parity_rcv;
  uint16_t            host_high;
  uint16_t            sensor_ack_low;
  uint16_t            sensor_ack_high;
  uint16_t            sensor_bit_starts[FW_LIB_DHT22_MAX_BIT_LEN];
  uint16_t            sensor_bits[FW_LIB_DHT22_MAX_BIT_LEN];
  uint16_t            bits[FW_LIB_DHT22_MAX_BIT_LEN];
  void*               gpio_handle;
  fw_lib_gpio_write_t cb_gpio_write;
  fw_lib_gpio_read_t  cb_gpio_read;
  fw_lib_delay_us_t   cb_delay_us;
} fw_lib_dht22;

#define FW_LIB_DHT22_RCV_NO_RESPONSE  (FW_LIB_ERROR + 1) // No response from sensor
#define FW_LIB_DHT22_RCV_BAD_ACK1     (FW_LIB_ERROR + 2) // Bad first half length of ACK impulse
#define FW_LIB_DHT22_RCV_BAD_ACK2     (FW_LIB_ERROR + 3) // Bad second half length of ACK impulse
#define FW_LIB_DHT22_RCV_RCV_TIMEOUT  (FW_LIB_ERROR + 4) // It was timeout while receiving bits

FW_LIB_DECLARE(void)     fw_lib_dht22_init(fw_lib_dht22 *handle, GPIO_TypeDef *hport, uint16_t pin_num);
FW_LIB_DECLARE(void)     fw_lib_dht22_init2(fw_lib_dht22 *handle,
                                            void* gpio_handle,
                                            fw_lib_gpio_write_t gpio_write_callback,
                                            fw_lib_gpio_read_t gpio_read_callback,
                                            fw_lib_delay_us_t delay_us_callback);
FW_LIB_DECLARE(uint32_t) fw_lib_dht22_get_readings(fw_lib_dht22 *handle);
FW_LIB_DECLARE(uint16_t) fw_lib_dht22_decode_readings(fw_lib_dht22 *handle);
FW_LIB_DECLARE(uint16_t) fw_lib_dht22_get_humidity(fw_lib_dht22 *handle);
FW_LIB_DECLARE(uint16_t) fw_lib_dht22_get_temperature(fw_lib_dht22 *handle);
FW_LIB_DECLARE(void)     fw_lib_dht22_gather_timing_data(fw_lib_dht22 *handle);

FW_LIB_END_DECLS

#endif
