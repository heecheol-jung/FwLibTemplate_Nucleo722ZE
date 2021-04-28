// Firmware library for STM32
// fl_stm32.h

#ifndef FL_STM32_H
#define FL_STM32_H

#include "main.h"

typedef struct _fl_stm32_gpio_handle
{
  GPIO_TypeDef*       hport;
  uint16_t            pin_num;
} fl_stm32_gpio_handle;

#endif
