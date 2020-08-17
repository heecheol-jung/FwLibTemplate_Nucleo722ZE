#include <stdio.h>
#include <string.h>
#include "fw_app.h"

FW_LIB_DECLARE_DATA fw_app_t g_app;

#if FW_APP_PARSER_CALLBACK == 1
static void on_message_parsed(const void* parser_handle, void* context);
#endif

FW_LIB_DECLARE(void) fw_app_init(void)
{
  int i;

  memset(&g_app, 0, sizeof(g_app));

  // Serial port for message communication.
  g_app.proto_mgr.uart_handle = &huart3;
#if FW_APP_PARSER_CALLBACK == 1
  g_app.proto_mgr.parser_handle.on_parsed_callback = on_message_parsed;
  g_app.proto_mgr.parser_handle.context = (void*)&g_app;
#endif

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

  // TODO : Device id setting(DIP switch, flash storage, ...).
  g_app.device_id = 1;

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
  // Do some work every 1 second.
  if (g_app.tick >= FW_APP_ONE_SEC_INTERVAL)
  {
    // LED1 toggle.
    HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
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

FW_LIB_DECLARE(fw_lib_dio_port_t*) fw_app_get_dout_port(uint8_t port_id)
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

FW_LIB_DECLARE(fw_lib_dio_port_t*) fw_app_get_din_port(uint8_t port_id)
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

#if FW_APP_PARSER_CALLBACK == 1
#if FW_APP_PARSER == FW_APP_TXT_PARSER
static void on_message_parsed(const void* parser_handle, void* context)
{
  fw_lib_txt_parser_t*    txt_parser = (fw_lib_txt_parser_t*)parser_handle;
  fw_app_proto_manager_t* proto_mgr = &((fw_app_t*)context)->proto_mgr;
  fw_lib_bool_t           cmd_processed = FW_LIB_FALSE;

  // Ignore the parsed message.
  if (txt_parser->device_id != ((fw_app_t*)context)->device_id)
  {
    return;
  }

  switch (txt_parser->msg_id)
  {
  case FW_LIB_MSG_ID_READ_HW_VERSION:
    proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d,%d.%d.%d%c",
        FW_LIB_TXT_RHVER_STR,
        txt_parser->device_id,
        FW_LIB_OK,
        FW_APP_HW_MAJOR, FW_APP_HW_MINOR, FW_APP_HW_REVISION,
        FW_LIB_TXT_MSG_TAIL);
    break;

  case FW_LIB_MSG_ID_READ_FW_VERSION:
    proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d,%d.%d.%d%c",
        FW_LIB_TXT_RFVER_STR,
        txt_parser->device_id,
        FW_LIB_OK,
        FW_APP_FW_MAJOR, FW_APP_FW_MINOR, FW_APP_FW_REVISION,
        FW_LIB_TXT_MSG_TAIL);
    break;

  case FW_LIB_MSG_ID_READ_GPIO:
    // Check the number of arguments.
    if (txt_parser->arg_count == 1)
    {
      // Check port number range.
      if ((txt_parser->args[0].value.uint8_value >= FW_APP_DIN_MIN_PORT_NUM) &&
          (txt_parser->args[0].value.uint8_value <= FW_APP_DIN_MAX_PORT_NUM))
      {
        fw_lib_dio_port_t* port = fw_app_get_din_port(txt_parser->args[0].value.uint8_value);
        fw_lib_bool_t din_value;

        if ((port != NULL) &&
            (fw_lib_dio_read(port, &din_value) == FW_LIB_OK))
        {
          proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d,%d,%d%c",
              FW_LIB_TXT_RGPIO_STR,
              txt_parser->device_id,
              FW_LIB_OK,
              txt_parser->args[0].value.uint8_value,
              din_value,
              FW_LIB_TXT_MSG_TAIL);
          cmd_processed = FW_LIB_TRUE;
        }
      }
    }

    if (cmd_processed != FW_LIB_TRUE)
    {
      proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d%c",
          FW_LIB_TXT_RGPIO_STR,
          txt_parser->device_id,
          FW_LIB_ERROR,
          FW_LIB_TXT_MSG_TAIL);
    }
    break;

  case FW_LIB_MSG_ID_WRITE_GPIO:
    // Check the number of arguments.
    if (txt_parser->arg_count == 2)
    {
      // Check port number range.
      if ((txt_parser->args[0].value.uint8_value >= FW_APP_DOUT_MIN_PORT_NUM) &&
          (txt_parser->args[0].value.uint8_value <= FW_APP_DOUT_MAX_PORT_NUM))
      {
        // Check port value.
        if ((txt_parser->args[1].value.uint8_value == FW_LIB_TRUE) ||
            (txt_parser->args[1].value.uint8_value == FW_LIB_FALSE))
        {
          fw_lib_dio_port_t* port = fw_app_get_dout_port(txt_parser->args[0].value.uint8_value);
          if (port != NULL)
          {
            if (fw_lib_dio_write(port, txt_parser->args[1].value.uint8_value) == FW_LIB_OK)
            {
              proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d%c",
                  FW_LIB_TXT_WGPIO_STR,
                  txt_parser->device_id,
                  FW_LIB_OK,
                  FW_LIB_TXT_MSG_TAIL);
              cmd_processed = FW_LIB_TRUE;
            }
          }
        }
      }
    }

    if (cmd_processed != FW_LIB_TRUE)
    {
      proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d%c",
          FW_LIB_TXT_WGPIO_STR,
          txt_parser->device_id,
          FW_LIB_ERROR,
          FW_LIB_TXT_MSG_TAIL);
    }
    break;
  }

  if (proto_mgr->out_length > 0)
  {
    HAL_UART_Transmit(proto_mgr->uart_handle, proto_mgr->out_buf, proto_mgr->out_length, FW_APP_PROTO_TX_TIMEOUT);
  }
  proto_mgr->out_length = 0;
}
#else
static void on_message_parsed(const void* parser_handle, void* context)
{
  fw_lib_bin_parser_t*      bin_parser = (fw_lib_bin_parser_t*)parser_handle;
  fw_app_proto_manager_t*   proto_mgr = &((fw_app_t*)context)->proto_mgr;
  fw_lib_bin_msg_header_t*  header = (fw_lib_bin_msg_header_t*)&bin_parser->buf[1];
  fw_lib_bool_t             cmd_processed = FW_LIB_FALSE;

  // Ignore the parsed message.
  if (header->device_id != ((fw_app_t*)context)->device_id)
  {
    return;
  }

  switch (header->message_id)
  {
    case FW_LIB_MSG_ID_READ_HW_VERSION:
    {
      fw_bin_msg_read_hw_ver_resp_t* resp = (fw_bin_msg_read_hw_ver_resp_t*)&proto_mgr->out_buf[1];
      resp->major = FW_APP_HW_MAJOR;
      resp->minor = FW_APP_HW_MINOR;
      resp->revision = FW_APP_HW_REVISION;
      proto_mgr->out_length = fw_lib_bin_msg_build_response(header->device_id, header->message_id, FW_LIB_BIT_FIELD_GET(header->flag1, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS), FW_LIB_FALSE, FW_LIB_OK, proto_mgr->out_buf);
      break;
    }

    case FW_LIB_MSG_ID_READ_FW_VERSION:
    {
      fw_bin_msg_read_fw_ver_resp_t* resp = (fw_bin_msg_read_fw_ver_resp_t*)&proto_mgr->out_buf[1];
      resp->major = FW_APP_FW_MAJOR;
      resp->minor = FW_APP_FW_MINOR;
      resp->revision = FW_APP_FW_REVISION;
      proto_mgr->out_length = fw_lib_bin_msg_build_response(header->device_id, header->message_id, FW_LIB_BIT_FIELD_GET(header->flag1, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS), FW_LIB_FALSE, FW_LIB_OK, proto_mgr->out_buf);
      break;
    }

    case FW_LIB_MSG_ID_WRITE_GPIO:
    {
      fw_bin_msg_write_gpio_cmd_t* cmd = (fw_bin_msg_write_gpio_cmd_t*)&proto_mgr->parser_handle.buf[1];
      if ((cmd->port_number >= FW_APP_DOUT_MIN_PORT_NUM) &&
          (cmd->port_number <= FW_APP_DOUT_MAX_PORT_NUM))
      {
        if ((cmd->port_value == 0) ||
            (cmd->port_value == 1))
        {
          fw_lib_dio_port_t* port = fw_app_get_dout_port(cmd->port_number);
          if (port != NULL)
          {
            if (fw_lib_dio_write(port, cmd->port_value) == FW_LIB_OK)
            {
              proto_mgr->out_length = fw_lib_bin_msg_build_response(header->device_id, header->message_id, FW_LIB_BIT_FIELD_GET(header->flag1, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS), FW_LIB_FALSE, FW_LIB_OK, proto_mgr->out_buf);
              cmd_processed = FW_LIB_TRUE;
            }
          }
        }
      }

      if (cmd_processed != FW_LIB_TRUE)
      {
        proto_mgr->out_length = fw_lib_bin_msg_build_response(header->device_id, header->message_id, FW_LIB_BIT_FIELD_GET(header->flag1, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS), FW_LIB_FALSE, FW_LIB_ERROR, proto_mgr->out_buf);
      }
      break;
    }

    case FW_LIB_MSG_ID_READ_GPIO:
    {
      fw_bin_msg_read_gpio_cmd_t* cmd = (fw_bin_msg_read_gpio_cmd_t*)&proto_mgr->parser_handle.buf[1];
      if ((cmd->port_number >= FW_APP_DIN_MIN_PORT_NUM) &&
          (cmd->port_number <= FW_APP_DIN_MAX_PORT_NUM))
      {
        fw_lib_dio_port_t* port = fw_app_get_din_port(cmd->port_number);
        fw_lib_bool_t din_value;

        if (fw_lib_dio_read(port, &din_value) == FW_LIB_OK)
        {
          fw_bin_msg_read_gpio_resp_t* resp = (fw_bin_msg_read_gpio_resp_t*)&proto_mgr->out_buf[1];
          resp->port_value = din_value;
          proto_mgr->out_length = fw_lib_bin_msg_build_response(header->device_id, header->message_id, FW_LIB_BIT_FIELD_GET(header->flag1, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS), FW_LIB_FALSE, FW_LIB_OK, proto_mgr->out_buf);
          cmd_processed = FW_LIB_TRUE;
        }
      }

      if (cmd_processed != FW_LIB_TRUE)
      {
        proto_mgr->out_length = fw_lib_bin_msg_build_response(header->device_id, header->message_id, FW_LIB_BIT_FIELD_GET(header->flag1, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS), FW_LIB_FALSE, FW_LIB_ERROR, proto_mgr->out_buf);
      }
      break;
    }
  }

  if (proto_mgr->out_length > 0)
  {
    HAL_UART_Transmit(proto_mgr->uart_handle, proto_mgr->out_buf, proto_mgr->out_length, FW_APP_PROTO_TX_TIMEOUT);
  }
  proto_mgr->out_length = 0;
}
#endif
#endif
