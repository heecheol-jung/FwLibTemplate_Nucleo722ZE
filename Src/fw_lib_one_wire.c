/*
 * fw_lib_one_wire.c
 *
 *  Created on: Dec 30, 2020
 *      Author: hcjung
 */

#include "gpio.h"
#include "fw_lib_one_wire.h"

FW_LIB_DECLARE(void) fw_lib_ow_init(fw_lib_one_wire *handle, GPIO_TypeDef* gpio_handle, uint16_t gpio_pin)
{
  handle->gpio_handle = gpio_handle;
  handle->gpio_pin = gpio_pin;

  fw_lib_ow_output(handle);
  fw_lib_ow_high(handle);
  handle->cb_delay_us(1000);

  fw_lib_ow_low(handle);
  handle->cb_delay_us(1000);

  fw_lib_ow_high(handle);
  handle->cb_delay_us(2000);
}

FW_LIB_DECLARE(uint8_t) fw_lib_ow_reset(fw_lib_one_wire *handle)
{
  uint8_t i;

  /* Line low, and wait 480us */
  fw_lib_ow_low(handle);
  fw_lib_ow_output(handle);
  handle->cb_delay_us(480);
  handle->cb_delay_us(20);

  /* Release line and wait for 70us */
  fw_lib_ow_input(handle);
  handle->cb_delay_us(70);

  /* Check bit value */
  i = fw_lib_ow_read_bit(handle);
  handle->cb_delay_us(410);

  return i;
}

FW_LIB_DECLARE(uint8_t) fw_lib_ow_search(fw_lib_one_wire *handle, uint8_t command)
{
  uint8_t id_bit_number;
  uint8_t last_zero, rom_byte_number, search_result;
  uint8_t id_bit, cmp_id_bit;
  uint8_t rom_byte_mask, search_direction;

  /* Initialize for search */
  id_bit_number = 1;
  last_zero = 0;
  rom_byte_number = 0;
  rom_byte_mask = 1;
  search_result = 0;

  // if the last call was not the last one
  if (!handle->last_device_flag)
  {
    // 1-Wire reset
    if (fw_lib_ow_reset(handle))
    {
      /* Reset the search */
      handle->last_discrepancy = 0;
      handle->last_device_flag = 0;
      handle->last_family_discrepancy = 0;
      return 0;
    }

    // issue the search command
    fw_lib_ow_write_byte(handle, command);

    // loop to do the search
    do
    {
      // read a bit and its complement
      id_bit = fw_lib_ow_read_bit(handle);
      cmp_id_bit = fw_lib_ow_read_bit(handle);

      // check for no devices on 1-wire
      if ((id_bit == 1) && (cmp_id_bit == 1))
      {
        break;
      }
      else
      {
        // all devices coupled have 0 or 1
        if (id_bit != cmp_id_bit)
        {
          search_direction = id_bit;  // bit write value for search
        }
        else
        {
          // if this discrepancy if before the Last Discrepancy
          // on a previous next then pick the same as last time
          if (id_bit_number < handle->last_discrepancy)
          {
            search_direction = ((handle->rom_no[rom_byte_number] & rom_byte_mask) > 0);
          }
          else
          {
            // if equal to last pick 1, if not then pick 0
            search_direction = (id_bit_number == handle->last_discrepancy);
          }

          // if 0 was picked then record its position in LastZero
          if (search_direction == 0)
          {
            last_zero = id_bit_number;

            // check for Last discrepancy in family
            if (last_zero < 9)
            {
              handle->last_family_discrepancy = last_zero;
            }
          }
        }

        // set or clear the bit in the ROM byte rom_byte_number
        // with mask rom_byte_mask
        if (search_direction == 1)
        {
          handle->rom_no[rom_byte_number] |= rom_byte_mask;
        }
        else
        {
          handle->rom_no[rom_byte_number] &= ~rom_byte_mask;
        }

        // serial number search direction write bit
        fw_lib_ow_write_bit(handle, search_direction);

        // increment the byte counter id_bit_number
        // and shift the mask rom_byte_mask
        id_bit_number++;
        rom_byte_mask <<= 1;

        // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
        if (rom_byte_mask == 0)
        {
          //docrc8(rom_no[rom_byte_number]);  // accumulate the CRC
          rom_byte_number++;
          rom_byte_mask = 1;
        }
      }
    } while (rom_byte_number < 8);  // loop until through all ROM bytes 0-7

    // if the search was successful then
    if (!(id_bit_number < 65))
    {
      // search successful so set last_discrepancy,last_device_flag,search_result
      handle->last_discrepancy = last_zero;

      // check for last device
      if (handle->last_discrepancy == 0)
      {
        handle->last_device_flag = 1;
      }

      search_result = 1;
    }
  }

  // if no device found then reset counters so next 'search' will be like a first
  if (!search_result || !handle->rom_no[0])
  {
    handle->last_discrepancy = 0;
    handle->last_device_flag = 0;
    handle->last_family_discrepancy = 0;
    search_result = 0;
  }

  return search_result;

}
FW_LIB_DECLARE(void) fw_lib_ow_reset_search(fw_lib_one_wire *handle)
{
  /* Reset the search state */
  handle->last_discrepancy = 0;
  handle->last_device_flag = 0;
  handle->last_family_discrepancy = 0;
}

FW_LIB_DECLARE(uint8_t) fw_lib_ow_first(fw_lib_one_wire *handle)
{
  /* Reset search values */
  fw_lib_ow_reset_search(handle);

  /* Start with searching */
  return fw_lib_ow_search(handle, FW_LIB_OW_CMD_SEARCHROM);
}

FW_LIB_DECLARE(uint8_t) fw_lib_ow_next(fw_lib_one_wire *handle)
{
  /* Leave the search state alone */
  return fw_lib_ow_search(handle, FW_LIB_OW_CMD_SEARCHROM);
}

FW_LIB_DECLARE(void) fw_lib_ow_get_full_rom(fw_lib_one_wire *handle, uint8_t* first_idx)
{
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    *(first_idx + i) = handle->rom_no[i];
  }
}

FW_LIB_DECLARE(void) fw_lib_ow_select(fw_lib_one_wire *handle, uint8_t* addr)
{
  uint8_t i;

  fw_lib_ow_write_byte(handle, FW_LIB_OW_CMD_MATCHROM);

  for (i = 0; i < 8; i++)
  {
    fw_lib_ow_write_byte(handle, *(addr + i));
  }
}

FW_LIB_DECLARE(void) fw_lib_ow_select_with_pointer(fw_lib_one_wire *handle, uint8_t* rom)
{
  uint8_t i;
  fw_lib_ow_write_byte(handle, FW_LIB_OW_CMD_MATCHROM);

  for (i = 0; i < 8; i++)
  {
    fw_lib_ow_write_byte(handle, *(rom + i));
  }
}

FW_LIB_DECLARE(uint8_t) fw_lib_ow_crc8(uint8_t *addr, uint8_t len)
{
  uint8_t crc = 0, inbyte, i, mix;

  while (len--)
  {
    inbyte = *addr++;
    for (i = 8; i; i--)
    {
      mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix)
      {
        crc ^= 0x8C;
      }
      inbyte >>= 1;
    }
  }

  /* Return calculated CRC */
  return crc;
}

FW_LIB_DECLARE(void)    fw_lib_ow_low(fw_lib_one_wire *handle)
{
  handle->gpio_handle->BSRR = handle->gpio_pin << 16;
}

FW_LIB_DECLARE(void)    fw_lib_ow_high(fw_lib_one_wire *handle)
{
  handle->gpio_handle->BSRR = handle->gpio_pin;
}

FW_LIB_DECLARE(void)    fw_lib_ow_input(fw_lib_one_wire *handle)
{
  GPIO_InitTypeDef  gpinit;
  gpinit.Mode = GPIO_MODE_INPUT;
  gpinit.Pull = GPIO_NOPULL;
  gpinit.Speed = GPIO_SPEED_FREQ_HIGH;
  gpinit.Pin = handle->gpio_pin;
  HAL_GPIO_Init(handle->gpio_handle,&gpinit);
}

FW_LIB_DECLARE(void)    fw_lib_ow_output(fw_lib_one_wire *handle)
{
  GPIO_InitTypeDef  gpinit;
  gpinit.Mode = GPIO_MODE_OUTPUT_OD;
  gpinit.Pull = GPIO_NOPULL;
  gpinit.Speed = GPIO_SPEED_FREQ_HIGH;
  gpinit.Pin = handle->gpio_pin;
  HAL_GPIO_Init(handle->gpio_handle,&gpinit);
}

FW_LIB_DECLARE(uint8_t) fw_lib_ow_read_byte(fw_lib_one_wire *handle)
{
  uint8_t i = 8, byte = 0;
  while (i--) {
    byte >>= 1;
    byte |= (fw_lib_ow_read_bit(handle) << 7);
  }

  return byte;
}

FW_LIB_DECLARE(uint8_t) fw_lib_ow_read_bit(fw_lib_one_wire *handle)
{
  uint8_t bit = 0;

  /* Line low */
  fw_lib_ow_low(handle);
  fw_lib_ow_output(handle);
  handle->cb_delay_us(2);

  /* Release line */
  fw_lib_ow_input(handle);
  handle->cb_delay_us(10);

  /* Read line value */
  if (HAL_GPIO_ReadPin(handle->gpio_handle, handle->gpio_pin)) {
    /* Bit is HIGH */
    bit = 1;
  }

  /* Wait 50us to complete 60us period */
  handle->cb_delay_us(50);

  /* Return bit value */
  return bit;
}

FW_LIB_DECLARE(void) fw_lib_ow_write_byte(fw_lib_one_wire *handle, uint8_t val)
{
  uint8_t i = 8;
  /* Write 8 bits */
  while (i--)
  {
    /* LSB bit is first */
    fw_lib_ow_write_bit(handle, val & 0x01);
    val >>= 1;
  }
}

FW_LIB_DECLARE(void) fw_lib_ow_write_bit(fw_lib_one_wire *handle, uint8_t bit)
{
  if (bit)
  {
    /* Set line low */
    fw_lib_ow_low(handle);
    fw_lib_ow_output(handle);
    handle->cb_delay_us(10);

    /* Bit high */
    fw_lib_ow_input(handle);

    /* Wait for 55 us and release the line */
    handle->cb_delay_us(55);
    fw_lib_ow_input(handle);
  }
  else
  {
    /* Set line low */
    fw_lib_ow_low(handle);
    fw_lib_ow_output(handle);
    handle->cb_delay_us(60);

    /* Bit high */
    fw_lib_ow_input(handle);

    /* Wait for 5 us and release the line */
    handle->cb_delay_us(5);
    fw_lib_ow_input(handle);
  }
}
