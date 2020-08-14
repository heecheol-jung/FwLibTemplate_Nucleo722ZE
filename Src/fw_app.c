#include <stdio.h>
#include <string.h>
#include "fw_app.h"

FW_LIB_DECLARE_DATA fw_app_t g_app;

FW_LIB_DECLARE(void) fw_app_init(void)
{
  int i;

  memset(&g_app, 0, sizeof(g_app));

  // Serial port for message communication.
  g_app.proto_mgr.uart_handle = &huart3;

  // GPIO inputs.
  for (i = 0; i < FW_APP_MAX_DIN; i++)
  {
    g_app.dins[i].port_id = i+1;
    g_app.dins[i].port_mode = FW_LIB_DIO_IN;
    g_app.dins[i].active_mode = FW_LIB_DIO_ACTIVE_HIGH;
  }
  g_app.dins[0].port = DIN1_GPIO_Port;
  g_app.dins[0].port_pin = DIN1_Pin;

  g_app.dins[1].port = DIN2_GPIO_Port;
  g_app.dins[1].port_pin = DIN2_Pin;

  // Button
  g_app.buttons[0].port_id = 1;
  g_app.buttons[0].port = USER_Btn_GPIO_Port;
  g_app.buttons[0].port_pin = USER_Btn_Pin;
  g_app.buttons[0].press_check_time = FW_APP_BTN_PRESS_CHECK_TIME;

  // GPIO outputs
  for (i = 0; i < FW_APP_MAX_DOUT; i++)
  {
    g_app.douts[i].port_id = i+1;
    g_app.douts[i].port_mode = FW_LIB_DIO_OUT;
    g_app.douts[i].active_mode = FW_LIB_DIO_ACTIVE_HIGH;
  }
  g_app.douts[0].port = LD2_GPIO_Port;
  g_app.douts[0].port_pin = LD2_Pin;

  g_app.douts[1].port = LD3_GPIO_Port;
  g_app.douts[1].port_pin = LD3_Pin;

  for (i = 0; i < FW_APP_BTN_COUNT; i++)
  {
    g_app.buttons[i].port_id = i+1;
    g_app.buttons[i].press_check_time = FW_APP_BTN_PRESS_CHECK_TIME;
  }
}

FW_LIB_DECLARE(void) fw_app_hw_init(void)
{
  int i;

  // GPIO output initial state.
  for (i = 0; i < FW_APP_MAX_DOUT; i++)
  {
    fw_lib_dio_write(&g_app.douts[i], FW_LIB_TRUE);
  }

  // Message receive in interrupt mode.
  FW_APP_UART_RCV_IT(g_app.proto_mgr.uart_handle, g_app.proto_mgr.rx_buf, 1);

  g_app.button_initialized = FW_LIB_TRUE;
}

FW_LIB_DECLARE(void) fw_app_systick(void)
{
  g_app.tick++;
  if (g_app.tick >= FW_APP_TICK_INTERVAL)
  {
    // LED1 toggle.
    HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
//    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
//    HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
    g_app.tick = 0;
  }

  // Physical button state update(debounce processing).
  if (g_app.button_initialized == FW_LIB_TRUE)
  {
    uint8_t i;

    for (i = 0; i < FW_APP_BTN_COUNT; i++)
    {
      // Physical button status : pressed
      if (HAL_GPIO_ReadPin(g_app.buttons[i].port, g_app.buttons[i].port_pin) == GPIO_PIN_SET)
      {
        if (g_app.buttons[i].prev_physical_status == FW_APP_BTN_STATE_RELEASED)
        {
          g_app.buttons[i].prev_physical_status = FW_APP_BTN_STATE_PRESSED;
        }

        // I am interested in pressed event.
        g_app.buttons[i].pressed_count++;
        if(g_app.buttons[i].pressed_count > g_app.buttons[i].press_check_time)
        {
          g_app.buttons[i].current_logical_status = FW_APP_BTN_STATE_PRESSED;
        }
      }
      // Physical button status : released
      else
      {
        if (g_app.buttons[i].prev_physical_status == FW_APP_BTN_STATE_PRESSED)
        {
          g_app.buttons[i].prev_physical_status = FW_APP_BTN_STATE_RELEASED;
          g_app.buttons[i].current_logical_status = FW_APP_BTN_STATE_RELEASED;
        }
      }
    }
  }
}
