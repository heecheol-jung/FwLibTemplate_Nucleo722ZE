/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include "fw_app.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int                     _i, _temp, _len;
uint8_t                 _ch;
fw_lib_status_t         _ret;
#if FW_APP_PARSER == FW_APP_TXT_PARSER
fw_lib_txt_msg_t        _parsed_msg;
fw_lib_msg_arg_t        _args[FW_LIB_TXT_MSG_MAX_ARG_COUNT];
#else
fw_lib_bin_msg_full_t   _parsed_msg;
#endif

fw_lib_bool_t           _cmd_processed;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static fw_lib_dio_port_t* get_din_port(uint8_t port_id);
static fw_lib_dio_port_t* get_dout_port(uint8_t port_id);
#if FW_APP_PARSER == FW_APP_TXT_PARSER
static void txt_message_processing(void);
#else
static void bin_message_processing(void);
#endif
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  fw_app_init();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  /* USER CODE BEGIN 2 */
  fw_app_hw_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // Update logical button state.
    for (_i = 0; _i < FW_APP_BTN_COUNT; _i++)
    {
      if (g_app.buttons[_i].prev_logical_status == FW_APP_BTN_STATE_RELEASED)
      {
        if (g_app.buttons[_i].current_logical_status == FW_APP_BTN_STATE_PRESSED)
        {
          // Send pressed event.
          // Button number.
#if FW_APP_PARSER == FW_APP_TXT_PARSER
          _args[0].type = FW_LIB_ARG_TYPE_UINT8;
          _args[0].value.uint8_value = 1;

          // BUtton value.
          _args[1].type = FW_LIB_ARG_TYPE_UINT8;
          _args[1].value.uint8_value = FW_LIB_BUTTON_PRESSED;
          _len = fw_lib_txt_msg_build_event(1, FW_LIB_MSG_ID_BUTTON_EVENT, _args, g_app.proto_mgr.out_buf);
#else
          fw_bin_msg_button_evt_t* evt = (fw_bin_msg_button_evt_t*)&g_app.proto_mgr.out_buf[1];
          evt->button_number = 1;
          evt->button_status = FW_APP_BTN_STATE_PRESSED;
          _len = fw_lib_bin_msg_build_event(1, FW_LIB_MSG_ID_BUTTON_EVENT, 0, g_app.proto_mgr.out_buf);
#endif
          if (_len > 0)
          {
            HAL_UART_Transmit(g_app.proto_mgr.uart_handle, g_app.proto_mgr.out_buf, _len, 500);
          }

          g_app.buttons[_i].prev_logical_status = FW_APP_BTN_STATE_PRESSED;
        }
      }
      else if (g_app.buttons[_i].prev_logical_status == FW_APP_BTN_STATE_PRESSED)
      {
        if (g_app.buttons[_i].current_logical_status == FW_APP_BTN_STATE_RELEASED)
        {
          g_app.buttons[_i].prev_logical_status = FW_APP_BTN_STATE_RELEASED;
        }
      }
    }

#if FW_APP_PARSER == FW_APP_TXT_PARSER
    txt_message_processing();
#else
    bin_message_processing();
#endif
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure LSE Drive Capability 
  */
  HAL_PWR_EnableBkUpAccess();
  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
#if FW_APP_PARSER == FW_APP_TXT_PARSER
static void txt_message_processing(void)
{
  _temp = fw_lib_q_count(&g_app.proto_mgr.q);
  if (_temp > 0)
  {
    for (_i = 0; _i < _temp; _i++)
    {
      fw_lib_q_pop(&g_app.proto_mgr.q, &_ch);
      _ret = fw_lib_txt_parser_parse_command(&g_app.proto_mgr.parser_handle, _ch, &_parsed_msg);
      if (_ret == FW_LIB_OK)
      {
        _len = 0;
        _cmd_processed = FW_LIB_FALSE;

        switch (_parsed_msg.msg_id)
        {
        case FW_LIB_MSG_ID_READ_HW_VERSION:
          // Version string.
          _args[0].type = FW_LIB_ARG_TYPE_STRING;
          sprintf(_args[0].value.string_value, "%d.%d.%d", FW_APP_HW_MAJOR, FW_APP_HW_MINOR, FW_APP_HW_REVISION);
          _len = fw_lib_txt_msg_build_response(_parsed_msg.device_id, FW_LIB_MSG_ID_READ_HW_VERSION, _args, FW_LIB_OK, g_app.proto_mgr.out_buf);
          break;

        case FW_LIB_MSG_ID_READ_FW_VERSION:
          // Version string.
          _args[0].type = FW_LIB_ARG_TYPE_STRING;
          sprintf(_args[0].value.string_value, "%d.%d.%d", FW_APP_FW_MAJOR, FW_APP_FW_MINOR, FW_APP_FW_REVISION);
          _len = fw_lib_txt_msg_build_response(_parsed_msg.device_id, FW_LIB_MSG_ID_READ_FW_VERSION, _args, FW_LIB_OK, g_app.proto_mgr.out_buf);
          break;

        case FW_LIB_MSG_ID_READ_GPIO:
          if (_parsed_msg.arg_count == 1)
          {
            uint8_t port_number = _parsed_msg.args[0].value.uint8_value;

            fw_lib_dio_port_t* port = get_din_port(port_number);
            fw_lib_bool_t din_value;

            if (fw_lib_dio_read(port, &din_value) == FW_LIB_OK)
            {
              // GPIO number.
              _args[0].type = FW_LIB_ARG_TYPE_UINT8;
              _args[0].value.uint8_value = port_number;

              // GPIO value.
              _args[1].type = FW_LIB_ARG_TYPE_UINT8;
              _args[1].value.uint8_value = din_value;

              _len = fw_lib_txt_msg_build_response(_parsed_msg.device_id, FW_LIB_MSG_ID_READ_GPIO, _args, FW_LIB_OK, g_app.proto_mgr.out_buf);
              _cmd_processed = FW_LIB_TRUE;
            }
          }

          if (_cmd_processed != FW_LIB_TRUE)
          {
            _len = fw_lib_txt_msg_build_response(_parsed_msg.device_id, FW_LIB_MSG_ID_READ_GPIO, NULL, FW_LIB_ERROR, g_app.proto_mgr.out_buf);
          }
          break;

        case FW_LIB_MSG_ID_WRITE_GPIO:
          if (_parsed_msg.arg_count == 2)
          {
            fw_lib_dio_port_t* port = get_dout_port(_parsed_msg.args[0].value.uint8_value);
            if (port != NULL)
            {
              if (fw_lib_dio_write(port, _parsed_msg.args[1].value.uint8_value) == FW_LIB_OK)
              {
                _len = fw_lib_txt_msg_build_response(_parsed_msg.device_id, FW_LIB_MSG_ID_WRITE_GPIO, NULL, FW_LIB_OK, g_app.proto_mgr.out_buf);
                _cmd_processed = FW_LIB_TRUE;
              }
            }
          }

          if (_cmd_processed != FW_LIB_TRUE)
          {
            _len = fw_lib_txt_msg_build_response(_parsed_msg.device_id, FW_LIB_MSG_ID_WRITE_GPIO, NULL, FW_LIB_ERROR, g_app.proto_mgr.out_buf);
          }
          break;
        }

        if (_len > 0)
        {
          HAL_UART_Transmit(g_app.proto_mgr.uart_handle, g_app.proto_mgr.out_buf, _len, 500);
        }
      }
    }
  }
}
#else
static void bin_message_processing(void)
{
  _temp = fw_lib_q_count(&g_app.proto_mgr.q);
  if (_temp > 0)
  {
    for (_i = 0; _i < _temp; _i++)
    {
      fw_lib_q_pop(&g_app.proto_mgr.q, &_ch);
      _ret = fw_lib_bin_parser_parse(&g_app.proto_mgr.parser_handle, _ch, &_parsed_msg);
      if (_ret == FW_LIB_OK)
      {
        _len = 0;
        _cmd_processed = FW_LIB_FALSE;
        switch (_parsed_msg.header.message_id)
        {
          case FW_LIB_MSG_ID_READ_HW_VERSION:
          {
            fw_bin_msg_read_hw_ver_resp_t* resp = (fw_bin_msg_read_hw_ver_resp_t*)&g_app.proto_mgr.out_buf[1];
            resp->major = FW_APP_HW_MAJOR;
            resp->minor = FW_APP_HW_MINOR;
            resp->revision = FW_APP_HW_REVISION;
            _len = fw_lib_bin_msg_build_response(_parsed_msg.header.device_id, _parsed_msg.header.message_id, FW_LIB_BIT_FIELD_GET(_parsed_msg.header.flag1, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS), FW_LIB_FALSE, FW_LIB_OK, g_app.proto_mgr.out_buf);
            break;
          }


          case FW_LIB_MSG_ID_READ_FW_VERSION:
          {
            fw_bin_msg_read_fw_ver_resp_t* resp = (fw_bin_msg_read_fw_ver_resp_t*)&g_app.proto_mgr.out_buf[1];
            resp->major = FW_APP_FW_MAJOR;
            resp->minor = FW_APP_FW_MINOR;
            resp->revision = FW_APP_FW_REVISION;
            _len = fw_lib_bin_msg_build_response(_parsed_msg.header.device_id, _parsed_msg.header.message_id, FW_LIB_BIT_FIELD_GET(_parsed_msg.header.flag1, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS), FW_LIB_FALSE, FW_LIB_OK, g_app.proto_mgr.out_buf);
            break;
          }

          case FW_LIB_MSG_ID_WRITE_GPIO:
          {
            fw_bin_msg_write_gpio_cmd_t* cmd = (fw_bin_msg_write_gpio_cmd_t*)&_parsed_msg.header;
            if ((cmd->port_number >= FW_APP_DOUT_MIN_PORT_NUM) && (cmd->port_number <= FW_APP_DOUT_MAX_PORT_NUM))
            {
              if ((cmd->port_value == 0) || (cmd->port_value == 1))
              {
                fw_lib_dio_port_t* port = get_dout_port(cmd->port_number);
                if (port != NULL)
                {
                  if (fw_lib_dio_write(port, cmd->port_value) == FW_LIB_OK)
                  {
                    _len = fw_lib_bin_msg_build_response(_parsed_msg.header.device_id, _parsed_msg.header.message_id, FW_LIB_BIT_FIELD_GET(_parsed_msg.header.flag1, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS), FW_LIB_FALSE, FW_LIB_OK, g_app.proto_mgr.out_buf);
                    _cmd_processed = FW_LIB_TRUE;
                  }
                }
              }
            }

            if (_cmd_processed != FW_LIB_TRUE)
            {
              _len = fw_lib_bin_msg_build_response(_parsed_msg.header.device_id, _parsed_msg.header.message_id, FW_LIB_BIT_FIELD_GET(_parsed_msg.header.flag1, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS), FW_LIB_FALSE, FW_LIB_ERROR, g_app.proto_mgr.out_buf);
            }
            break;
          }

          case FW_LIB_MSG_ID_READ_GPIO:
          {
            fw_bin_msg_read_gpio_cmd_t* cmd = (fw_bin_msg_read_gpio_cmd_t*)&_parsed_msg.header;
            if ((cmd->port_number >= FW_APP_DIN_MIN_PORT_NUM) && (cmd->port_number <= FW_APP_DIN_MAX_PORT_NUM))
            {
              fw_lib_dio_port_t* port = get_din_port(cmd->port_number);
              fw_lib_bool_t din_value;

              if (fw_lib_dio_read(port, &din_value) == FW_LIB_OK)
              {
                fw_bin_msg_read_gpio_resp_t* resp = (fw_bin_msg_read_gpio_resp_t*)&g_app.proto_mgr.out_buf[1];
                resp->port_value = din_value;
                _len = fw_lib_bin_msg_build_response(_parsed_msg.header.device_id, _parsed_msg.header.message_id, FW_LIB_BIT_FIELD_GET(_parsed_msg.header.flag1, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS), FW_LIB_FALSE, FW_LIB_OK, g_app.proto_mgr.out_buf);
                _cmd_processed = FW_LIB_TRUE;
              }
            }

            if (_cmd_processed != FW_LIB_TRUE)
            {
              _len = fw_lib_bin_msg_build_response(_parsed_msg.header.device_id, _parsed_msg.header.message_id, FW_LIB_BIT_FIELD_GET(_parsed_msg.header.flag1, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS), FW_LIB_FALSE, FW_LIB_ERROR, g_app.proto_mgr.out_buf);
            }
            break;
          }
        }

        if (_len > 0)
        {
          HAL_UART_Transmit(g_app.proto_mgr.uart_handle, g_app.proto_mgr.out_buf, _len, 500);
        }
      }

    }
  }
}
#endif

static fw_lib_dio_port_t* get_dout_port(uint8_t port_id)
{
  uint8_t i;

  for (i = 0; i < FW_APP_MAX_DOUT; i++)
  {
    if (g_app.douts[i].port_id == port_id)
    {
      return &g_app.douts[i];
    }
  }

  return NULL;
}

static fw_lib_dio_port_t* get_din_port(uint8_t port_id)
{
  uint8_t i;

  for (i = 0; i < FW_APP_MAX_DOUT; i++)
  {
    if (g_app.dins[i].port_id == port_id)
    {
      return &g_app.dins[i];
    }
  }

  return NULL;
}

// Callback function for UART data reception.
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == g_app.proto_mgr.uart_handle)
  {
    fw_lib_q_push(&g_app.proto_mgr.q, g_app.proto_mgr.rx_buf[0]);
    FW_APP_UART_RCV_IT(g_app.proto_mgr.uart_handle, g_app.proto_mgr.rx_buf, 1);
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
