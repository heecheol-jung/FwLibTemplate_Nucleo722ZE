#include <fl_dio.h>
#include "main.h"
#include "gpio.h"
#include "fl_def.h"

FL_DECLARE(uint8_t) fl_dio_write(fl_dio_port_t *port_handle, fl_bool_t logical_on_off)
{
  GPIO_PinState pin_value;

  if (port_handle->port_mode != FL_DIO_OUT)
  {
    return FL_ERROR;
  }

  if (port_handle->active_mode == FL_DIO_ACTIVE_HIGH)
  {
    if (logical_on_off == FL_TRUE)
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
    if (logical_on_off == FL_TRUE)
    {
      pin_value = GPIO_PIN_RESET;
    }
    else
    {
      pin_value = GPIO_PIN_SET;
    }
  }

  HAL_GPIO_WritePin(port_handle->port, port_handle->port_pin, pin_value);

  return FL_OK;
}

FL_DECLARE(uint8_t) fl_dio_read(fl_dio_port_t *port_handle, fl_bool_t *logical_on_off)
{
  GPIO_PinState pin_valule;

  if (port_handle->port_mode != FL_DIO_IN)
  {
    return FL_ERROR;
  }

  pin_valule = HAL_GPIO_ReadPin(port_handle->port, port_handle->port_pin);

  if (port_handle->active_mode == FL_DIO_ACTIVE_HIGH)
  {
    if (pin_valule == GPIO_PIN_SET)
    {
      *logical_on_off = FL_TRUE;
    }
    else
    {
      *logical_on_off = FL_FALSE;
    }
  }
  else
  {
    if (pin_valule == GPIO_PIN_RESET)
    {
      *logical_on_off = FL_TRUE;
    }
    else
    {
      *logical_on_off = FL_FALSE;
    }
  }

  return FL_OK;
}
