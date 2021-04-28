// Firmware library for DHT22
// fl_dht22.h

#ifndef FL_DHT22_H
#define FL_DHT22_H

// https://github.com/LonelyWolf/stm32/blob/master/am2302/dht22.h
// https://github.com/nixnodes/stm32-am2302/blob/master/dht22.h

#include "gpio.h"
#include "fl_def.h"
#include "fl_base_device.h"
#include "fl_stm32.h"

#define FL_DHT22_MAX_BIT_LEN  (40)

FL_BEGIN_DECLS

// Read interval : 2 seconds
// https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/#:~:text=The%20DHT22%20sensor%20has%20a%20better%20resolution%20and,However%2C%20you%20can%20request%20sensor%20readings%20every%20second.
typedef struct _fl_dht22
{
  fl_base_device  common_data;
  uint8_t         hmsb;
  uint8_t         hlsb;
  uint8_t         tmsb;
  uint8_t         tlsb;
  uint8_t         parity_rcv;
  uint16_t        host_high;
  uint16_t        sensor_ack_low;
  uint16_t        sensor_ack_high;
  uint16_t        sensor_bit_starts[FL_DHT22_MAX_BIT_LEN];
  uint16_t        sensor_bits[FL_DHT22_MAX_BIT_LEN];
  uint16_t        bits[FL_DHT22_MAX_BIT_LEN];
  void*           gpio_handle;
  fl_gpio_write_t cb_gpio_write;
  fl_gpio_read_t  cb_gpio_read;
  fl_delay_us_t   cb_delay_us;
} fl_dht22;

#define FL_DHT22_RCV_NO_RESPONSE  (FL_ERROR + 1) // No response from sensor
#define FL_DHT22_RCV_BAD_ACK1     (FL_ERROR + 2) // Bad first half length of ACK impulse
#define FL_DHT22_RCV_BAD_ACK2     (FL_ERROR + 3) // Bad second half length of ACK impulse
#define FL_DHT22_RCV_RCV_TIMEOUT  (FL_ERROR + 4) // It was timeout while receiving bits

FL_DECLARE(void)     fl_dht22_init(fl_dht22 *handle, GPIO_TypeDef *hport, uint16_t pin_num);
FL_DECLARE(void)     fl_dht22_init2(fl_dht22 *handle,
                                            void* gpio_handle,
                                            fl_gpio_write_t gpio_write_callback,
                                            fl_gpio_read_t gpio_read_callback,
                                            fl_delay_us_t delay_us_callback);
FL_DECLARE(uint32_t) fl_dht22_get_readings(fl_dht22 *handle);
FL_DECLARE(uint16_t) fl_dht22_decode_readings(fl_dht22 *handle);
FL_DECLARE(uint16_t) fl_dht22_get_humidity(fl_dht22 *handle);
FL_DECLARE(uint16_t) fl_dht22_get_temperature(fl_dht22 *handle);
FL_DECLARE(void)     fl_dht22_gather_timing_data(fl_dht22 *handle);

FL_END_DECLS

#endif
