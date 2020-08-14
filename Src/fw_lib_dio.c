#include "main.h"
#include "gpio.h"
#include "fw_lib_def.h"
#include "fw_lib_dio.h"

FW_LIB_DECLARE(uint8_t) fw_lib_dio_write(fw_lib_dio_port_t *port_handle, fw_lib_bool_t logical_on_off)
{
  GPIO_PinState pin_value;

  if (port_handle->port_mode != FW_LIB_DIO_OUT)
  {
    return FW_LIB_ERROR;
  }

  if (port_handle->active_mode == FW_LIB_DIO_ACTIVE_HIGH)
  {
    if (logical_on_off == FW_LIB_TRUE)
    {
      pin_value = GPIO_PIN_SET;
    }
    else
    {
      pin_value = GPIO_PIN_RESET;
    }
  }
  else
  {
    if (logical_on_off == FW_LIB_TRUE)
    {
      pin_value = GPIO_PIN_RESET;
    }
    else
    {
      pin_value = GPIO_PIN_SET;
    }
  }

  HAL_GPIO_WritePin(port_handle->port, port_handle->port_pin, pin_value);

  return FW_LIB_OK;
}

FW_LIB_DECLARE(uint8_t) fw_lib_dio_read(fw_lib_dio_port_t *port_handle, fw_lib_bool_t *logical_on_off)
{
  GPIO_PinState pin_valule;

  if (port_handle->port_mode != FW_LIB_DIO_IN)
  {
    return FW_LIB_ERROR;
  }

  pin_valule = HAL_GPIO_ReadPin(port_handle->port, port_handle->port_pin);

  if (port_handle->active_mode == FW_LIB_DIO_ACTIVE_HIGH)
  {
    if (pin_valule == GPIO_PIN_SET)
    {
      *logical_on_off = FW_LIB_TRUE;
    }
    else
    {
      *logical_on_off = FW_LIB_FALSE;
    }
  }
  else
  {
    if (pin_valule == GPIO_PIN_RESET)
    {
      *logical_on_off = FW_LIB_TRUE;
    }
    else
    {
      *logical_on_off = FW_LIB_FALSE;
    }
  }

  return FW_LIB_OK;
}
