#include <fl_dht22.h>
#include <fl_timer_delay.h>
#include <string.h>

FL_DECLARE(void) fl_dht22_init(fl_dht22 *handle, GPIO_TypeDef *hport, uint16_t pin_num)
{
  memset(handle, 0, sizeof(fl_dht22));
  handle->common_data.device_type = FL_DEVICE_TYPE_SENSOR;
}

FL_DECLARE(void) fl_dht22_init2(fl_dht22 *handle,
                                        void* gpio_handle,
                                        fl_gpio_write_t gpio_write_callback,
                                        fl_gpio_read_t gpio_read_callback,
                                        fl_delay_us_t delay_us_callback)
{
  memset(handle, 0, sizeof(fl_dht22));
  handle->common_data.device_type = FL_DEVICE_TYPE_SENSOR;
  handle->gpio_handle = gpio_handle;
  handle->cb_gpio_write = gpio_write_callback;
  handle->cb_gpio_read = gpio_read_callback;
  handle->cb_delay_us = delay_us_callback;
}

FL_DECLARE(void) fl_dht22_gather_timing_data(fl_dht22 *handle)
{
  uint32_t wait;
  uint8_t i;

  handle->host_high = 0;
  handle->sensor_ack_low = 0;
  handle->sensor_ack_high = 0;
  for (i = 0; i < FL_DHT22_MAX_BIT_LEN; i++)
  {
    handle->sensor_bit_starts[i] = 0;
    handle->sensor_bits[i] = 0;
  }

  HAL_NVIC_DisableIRQ(SysTick_IRQn);

  // Start
  // Generate start impulse for sensor
  handle->cb_gpio_write(handle->gpio_handle, FL_TRUE);
  handle->cb_gpio_write(handle->gpio_handle, FL_FALSE);

  handle->cb_delay_us(900);
  handle->cb_gpio_write(handle->gpio_handle, FL_TRUE);

  // Wait for AM2302 to start communicate
  wait = 0;
  while ((handle->cb_gpio_read(handle->gpio_handle) == FL_TRUE) && (wait++ < 200)) handle->cb_delay_us(1);
  handle->host_high = wait;

  // Check ACK strobe from sensor
  wait = 0;
  while ((handle->cb_gpio_read(handle->gpio_handle) == FL_FALSE) && (wait++ < 100)) handle->cb_delay_us(1);
  handle->sensor_ack_low = wait;

  wait = 0;
  while ((handle->cb_gpio_read(handle->gpio_handle) == FL_TRUE) && (wait++ < 100)) handle->cb_delay_us(1);
  handle->sensor_ack_high = wait;

  // ACK strobe received --> receive 40 bits
  // Byte1                                                     Byte2
  // Bit start Bit0 Bit start ... Bit start Bit7               Bit start Bit0 ... Bit start Bit7
  // 54us           54us          54us           13.6us Dealy  54us               54us
  //                                             |67.6 us                    |
  i = 0;
  while (i < FL_DHT22_MAX_BIT_LEN)
  {
    // Measure bit start impulse (T_low = 50us), Actual = 54us, 67.6us(Byte start for 2nd, 3rd, 4th, 5th byte)
    wait = 0;
    while ((handle->cb_gpio_read(handle->gpio_handle) == FL_FALSE) && (wait++ < 100)) handle->cb_delay_us(1);
    handle->sensor_bit_starts[i] = wait;

    // Measure bit impulse length (T_h0 = 25us, T_h1 = 70us)
    wait = 0;
    //while ((handle->hport->IDR & DHT22_GPIO_PIN) && (wait++ < 20)) handle->cb_delay_us(1);
    while ((handle->cb_gpio_read(handle->gpio_handle) == FL_TRUE) && (wait++ < 100)) handle->cb_delay_us(1);
    handle->sensor_bits[i] = wait;

    i++;
  }
  // End
  // End - Start = 6milliseconds.

  HAL_NVIC_EnableIRQ(SysTick_IRQn);
}

FL_DECLARE(uint32_t) fl_dht22_get_readings(fl_dht22 *handle)
{
  uint32_t wait;
  uint8_t i;

  handle->host_high = 0;
  handle->sensor_ack_low = 0;
  handle->sensor_ack_high = 0;
  for (i = 0; i < FL_DHT22_MAX_BIT_LEN; i++)
  {
    handle->sensor_bit_starts[i] = 0;
    handle->sensor_bits[i] = 0;
  }

  // https://www.teachmemicro.com/how-dht22-sensor-works/
  // Generate start impulse for sensor
  // Pull down SDA (Bit_SET)
  handle->cb_gpio_write(handle->gpio_handle, FL_TRUE);
  // Release SDA (Bit_RESET)
  handle->cb_gpio_write(handle->gpio_handle, FL_FALSE);
  handle->cb_delay_us(800); // Host start signal at least 800us
  handle->cb_gpio_write(handle->gpio_handle, FL_TRUE);

  // Wait for AM2302 to start communicate
  wait = 0;
  while ((handle->cb_gpio_read(handle->gpio_handle) == FL_TRUE) && (wait++ < 200)) handle->cb_delay_us(1);
  // If a processor is changed, increase or decrease wait value.
  if (wait > 12)  // Min : 10, Max : 10
  {
    return FL_DHT22_RCV_NO_RESPONSE; // Host high : 9unit(measured value) 11unit (with margin).
  }

  // Check ACK strobe from sensor
  wait = 0;
  while ((handle->cb_gpio_read(handle->gpio_handle) == FL_FALSE) && (wait++ < 100)) handle->cb_delay_us(1);
  // If a processor is changed, increase or decrease wait value.
  if ((wait < 25) || (wait > 37)) // Min : 31, Max 32
  {
    return FL_DHT22_RCV_BAD_ACK1; // Sensor ACK low : 48unit, minimum : 44unit(with margin), maximum : 53unit(with margin).
  }

  wait = 0;
  while ((handle->cb_gpio_read(handle->gpio_handle) == FL_TRUE) && (wait++ < 100)) handle->cb_delay_us(1);
  // If a processor is changed, increase or decrease wait value.
  if ((wait < 25) || (wait > 37)) // Min : 30, Max : 32
  {
    return FL_DHT22_RCV_BAD_ACK2; // Sensor ACK high : 51unit, minimum : 46unit(with margin, maximum : 56unit(with margin).
  }

  // ACK strobe received --> receive 40 bits
  i = 0;
  while (i < FL_DHT22_MAX_BIT_LEN)
  {
    // Measure bit start impulse (T_low = 50us), 33~42unit
    // T_low : 33unit
    wait = 0;
    while ((handle->cb_gpio_read(handle->gpio_handle) == FL_FALSE) && (wait++ < 100)) handle->cb_delay_us(1);
    handle->sensor_bit_starts[i] = wait;
    if (wait > 32) // Min : 20, Max : 27
    {
      // invalid bit start impulse length
      handle->bits[i] = 0xffff;
      while ((handle->cb_gpio_read(handle->gpio_handle) == FL_TRUE) && (wait++ < 100)) handle->cb_delay_us(1);
    }
    else
    {
      // Min : 9, Max : 29
      // Measure bit impulse length (T_h0 = 25us, T_h1 = 70us), 15~46unit
      // T_h0 : 15~17unit, T_h1 : 45~46unit
      wait = 0;
      while ((handle->cb_gpio_read(handle->gpio_handle) == FL_TRUE) && (wait++ < 100)) handle->cb_delay_us(1);
      handle->sensor_bits[i] = wait;
      handle->bits[i] = (wait < 36) ? wait : 0xffff;
    }

    i++;
  }

  for (i = 0; i < FL_DHT22_MAX_BIT_LEN; i++) if (handle->bits[i] == 0xffff) return FL_DHT22_RCV_RCV_TIMEOUT;

  return FL_OK;
}

FL_DECLARE(uint16_t) fl_dht22_decode_readings(fl_dht22 *handle)
{
  uint8_t parity;
  uint8_t  i = 0;

  handle->hmsb = 0;
  for (; i < 8; i++)
  {
    handle->hmsb <<= 1;
    if (handle->bits[i] > 18) handle->hmsb |= 1;
  }

  handle->hlsb = 0;
  for (; i < 16; i++)
  {
    handle->hlsb <<= 1;
    if (handle->bits[i] > 18) handle->hlsb |= 1;
  }

  handle->tmsb = 0;
  for (; i < 24; i++)
  {
    handle->tmsb <<= 1;
    if (handle->bits[i] > 18) handle->tmsb |= 1;
  }

  handle->tlsb = 0;
  for (; i < 32; i++)
  {
    handle->tlsb <<= 1;
    if (handle->bits[i] > 18) handle->tlsb |= 1;
  }

  for (; i < FL_DHT22_MAX_BIT_LEN; i++)
  {
    handle->parity_rcv <<= 1;
    if (handle->bits[i] > 18) handle->parity_rcv |= 1;
  }

  parity  = handle->hmsb + handle->hlsb + handle->tmsb + handle->tlsb;

  return (handle->parity_rcv << 8) | parity;
}

FL_DECLARE(uint16_t) fl_dht22_get_humidity(fl_dht22 *handle)
{
  return (handle->hmsb << 8) + handle->hlsb;
}

FL_DECLARE(uint16_t) fl_dht22_get_temperature(fl_dht22 *handle)
{
  return (handle->tmsb << 8) + handle->tlsb;
}

