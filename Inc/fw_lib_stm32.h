// fw_lib_stm32.h

#ifndef FW_LIB_STM32_H
#define FW_LIB_STM32_H

#include "main.h"

typedef struct _fw_lib_stm32_gpio_handle
{
  GPIO_TypeDef*       hport;
  uint16_t            pin_num;
} fw_lib_stm32_gpio_handle;

#endif
