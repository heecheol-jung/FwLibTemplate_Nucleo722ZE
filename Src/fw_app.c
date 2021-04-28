#include <stdio.h>
#include <string.h>
#include "fw_app.h"

extern TIM_HandleTypeDef htim2;

FL_DECLARE_DATA fw_app_t g_app;

#if FW_APP_PARSER_CALLBACK == 1
static void on_message_parsed(const void* parser_handle, void* context);
#endif

#if defined(FW_APP_PARSER_DEBUG)
static void on_parse_started(const void* parser_handle);
static void on_parse_ended(const void* parser_handle);
#endif

void _dht22_gpio_write(const void* gpio_handle, fl_bool_t on_off);
fl_bool_t _dht22_gpio_read(const void* gpio_handle);
void _dht22_delay_us(volatile uint32_t microseconds);

FL_DECLARE(void) fw_app_init(void)
{
  int i;

  memset(&g_app, 0, sizeof(g_app));

  // Serial port for message communication.
  g_app.proto_mgr.uart_handle = &huart3;
#if FW_APP_PARSER_CALLBACK == 1
  g_app.proto_mgr.parser_handle.on_parsed_callback = on_message_parsed;
  g_app.proto_mgr.parser_handle.context = (void*)&g_app;
#endif

#if defined(FW_APP_PARSER_DEBUG)
  g_app.proto_mgr.parser_handle.on_parse_started_callback = on_parse_started;
  g_app.proto_mgr.parser_handle.on_parse_ended_callback = on_parse_ended;
#endif

  // GPIO inputs.
  for (i = 0; i < FW_APP_MAX_DIN; i++)
  {
    g_app.dins[i].port_id = i+1;
    g_app.dins[i].port_mode = FL_DIO_IN;
    g_app.dins[i].active_mode = FL_DIO_ACTIVE_HIGH;
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
    g_app.douts[i].port_mode = FL_DIO_OUT;
    g_app.douts[i].active_mode = FL_DIO_ACTIVE_HIGH;
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

FL_DECLARE(void) fw_app_hw_init(void)
{
  int i;

  // TODO : Device id setting(DIP switch, flash storage, ...).
  g_app.device_id = 1;

  // GPIO output pin for debugging.
  HAL_GPIO_WritePin(DBG_OUT1_GPIO_Port, DBG_OUT1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DBG_OUT2_GPIO_Port, DBG_OUT2_Pin, GPIO_PIN_RESET);

  // GPIO output initial state.
  for (i = 0; i < FW_APP_MAX_DOUT; i++)
  {
    fl_dio_write(&g_app.douts[i], FL_TRUE);
  }

  // Initialize DHT22.
  g_app.dht22[0].dht22_gpio.hport = DHT22_GPIO_Port;
  g_app.dht22[0].dht22_gpio.pin_num = DHT22_Pin;
  fl_dht22_init2(&g_app.dht22[0].dht22_handle, &g_app.dht22[0].dht22_gpio, _dht22_gpio_write, _dht22_gpio_read, _dht22_delay_us);

  // Initialize DS18B20.
  g_app.ds18b20[0].one_wire.cb_delay_us = _dht22_delay_us;
  fl_ds18b20_init(&g_app.ds18b20[0], DS18B20_GPIO_Port, DS18B20_Pin);

  // Message receive in interrupt mode.
  FW_APP_UART_RCV_IT(g_app.proto_mgr.uart_handle, g_app.proto_mgr.rx_buf, 1);

  g_app.button_initialized = FL_TRUE;
}

FL_DECLARE(void) fw_app_systick(void)
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
  if (g_app.button_initialized == FL_TRUE)
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

FL_DECLARE(fl_dio_port_t*) fw_app_get_dout_port(uint8_t port_id)
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

FL_DECLARE(fl_dio_port_t*) fw_app_get_din_port(uint8_t port_id)
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
  fl_txt_msg_parser_t*    txt_parser = (fl_txt_msg_parser_t*)parser_handle;
  fw_app_proto_manager_t* proto_mgr = &((fw_app_t*)context)->proto_mgr;
  fl_bool_t           cmd_processed = FL_FALSE;
  fl_status_t         ret;

  // Ignore the parsed message.
  if (txt_parser->device_id != ((fw_app_t*)context)->device_id)
  {
    return;
  }

  switch (txt_parser->msg_id)
  {
  case FL_MSG_ID_READ_HW_VERSION:
    proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d,%d.%d.%d%c",
        fl_txt_msg_get_message_name(txt_parser->msg_id),
        txt_parser->device_id,
        FL_OK,
        FW_APP_HW_MAJOR, FW_APP_HW_MINOR, FW_APP_HW_REVISION,
        FL_TXT_MSG_TAIL);
    break;

  case FL_MSG_ID_READ_FW_VERSION:
    proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d,%d.%d.%d%c",
        fl_txt_msg_get_message_name(txt_parser->msg_id),
        txt_parser->device_id,
        FL_OK,
        FW_APP_FW_MAJOR, FW_APP_FW_MINOR, FW_APP_FW_REVISION,
        FL_TXT_MSG_TAIL);
    break;

  case FL_MSG_ID_READ_GPIO:
    // Check the number of arguments.
    if (txt_parser->arg_count == 1)
    {
      fl_gpi_port_t* gpi_port = (fl_gpi_port_t*)&(proto_mgr->parser_handle.payload);
      // Check port number range.
      if ((gpi_port->port_num >= FW_APP_DIN_MIN_PORT_NUM) &&
          (gpi_port->port_num <= FW_APP_DIN_MAX_PORT_NUM))
      {
        fl_dio_port_t* port = fw_app_get_din_port(gpi_port->port_num);
        fl_bool_t din_value;

        if ((port != NULL) &&
            (fl_dio_read(port, &din_value) == FL_OK))
        {
          proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d,%d,%d%c",
              fl_txt_msg_get_message_name(txt_parser->msg_id),
              txt_parser->device_id,
              FL_OK,
              gpi_port->port_num,
              din_value,
              FL_TXT_MSG_TAIL);
          cmd_processed = FL_TRUE;
        }
      }
    }

    if (cmd_processed != FL_TRUE)
    {
      proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d%c",
          fl_txt_msg_get_message_name(txt_parser->msg_id),
          txt_parser->device_id,
          FL_ERROR,
          FL_TXT_MSG_TAIL);
    }
    break;

  case FL_MSG_ID_WRITE_GPIO:
    // Check the number of arguments.
    if (txt_parser->arg_count == 2)
    {
      fl_gpo_port_value_t* gpo_port_value = (fl_gpo_port_value_t*)&(proto_mgr->parser_handle.payload);

      // Check port number range.
      if ((gpo_port_value->port_num >= FW_APP_DOUT_MIN_PORT_NUM) &&
          (gpo_port_value->port_num <= FW_APP_DOUT_MAX_PORT_NUM))
      {
        // Check port value.
        if ((gpo_port_value->port_value == FL_TRUE) ||
            (gpo_port_value->port_value == FL_FALSE))
        {
          fl_dio_port_t* port = fw_app_get_dout_port(gpo_port_value->port_num);
          if (port != NULL)
          {
            if (fl_dio_write(port, gpo_port_value->port_value) == FL_OK)
            {
              proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d%c",
                  fl_txt_msg_get_message_name(txt_parser->msg_id),
                  txt_parser->device_id,
                  FL_OK,
                  FL_TXT_MSG_TAIL);
              cmd_processed = FL_TRUE;
            }
          }
        }
      }
    }

    if (cmd_processed != FL_TRUE)
    {
      proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d%c",
          fl_txt_msg_get_message_name(txt_parser->msg_id),
          txt_parser->device_id,
          FL_ERROR,
          FL_TXT_MSG_TAIL);
    }
    break;

  case FL_MSG_ID_READ_TEMPERATURE:
    // Check the number of arguments.
    if (txt_parser->arg_count == 1)
    {
      fl_sensor_t* sensor = (fl_sensor_t*)&(proto_mgr->parser_handle.payload);

      // Check sensor number.
      if (sensor->sensor_num == 1)
      {
        ret = fl_dht22_get_readings(&g_app.dht22[0].dht22_handle);
        if (ret == FL_OK)
        {
          uint16_t temperature = 0;

          fl_dht22_decode_readings(&g_app.dht22[0].dht22_handle);
          temperature = fl_dht22_get_temperature(&g_app.dht22[0].dht22_handle);

          proto_mgr->out_length = sprintf((char*)g_app.proto_mgr.out_buf, "%s %ld,%d,%d,%d.%d%c",
              fl_txt_msg_get_message_name(txt_parser->msg_id),
              txt_parser->device_id,
              FL_OK,
              sensor->sensor_num,
              (temperature & 0xffff) / 10,
              (temperature & 0x7fff) % 10,
              FL_TXT_MSG_TAIL);

          cmd_processed = FL_TRUE;
        }
      }
      else if (sensor->sensor_num == 2)
      {
        fl_ds18b20_manual_convert(&g_app.ds18b20[0]);
        if (g_app.ds18b20[0].devices[0].data_valid == FL_TRUE)
        {
          proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d,%d,%.2f%c",
              fl_txt_msg_get_message_name(txt_parser->msg_id),
              txt_parser->device_id,
              FL_OK,
              sensor->sensor_num,
              g_app.ds18b20[0].devices[0].temperature,
              FL_TXT_MSG_TAIL);

          cmd_processed = FL_TRUE;
        }
      }
    }

    if (cmd_processed != FL_TRUE)
    {
      proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d%c",
          fl_txt_msg_get_message_name(txt_parser->msg_id),
          txt_parser->device_id,
          ret,
          FL_TXT_MSG_TAIL);
    }
    break;

  case FL_MSG_ID_READ_HUMIDITY:
    // Check the number of arguments.
    if (txt_parser->arg_count == 1)
    {
      fl_sensor_t* sensor = (fl_sensor_t*)&(proto_mgr->parser_handle.payload);

      // Check sensor number.
      if (sensor->sensor_num == FW_APP_DHT22_MIN_NUM)
      {
      ret = fl_dht22_get_readings(&g_app.dht22[0].dht22_handle);
        if (ret == FL_OK)
        {
          uint16_t humidity = 0;

          ret = fl_dht22_decode_readings(&g_app.dht22[0].dht22_handle);
          humidity = fl_dht22_get_humidity(&g_app.dht22[0].dht22_handle);

          proto_mgr->out_length = sprintf((char*)g_app.proto_mgr.out_buf, "%s %ld,%d,%d,%d.%d%c",
              fl_txt_msg_get_message_name(txt_parser->msg_id),
              txt_parser->device_id,
              FL_OK,
              sensor->sensor_num,
              humidity / 10,
              humidity % 10,
              FL_TXT_MSG_TAIL);

          cmd_processed = FL_TRUE;
        }
      }
    }

    if (cmd_processed != FL_TRUE)
    {
      proto_mgr->out_length = sprintf((char*)proto_mgr->out_buf, "%s %ld,%d%c",
          fl_txt_msg_get_message_name(txt_parser->msg_id),
          txt_parser->device_id,
          FL_ERROR,
          FL_TXT_MSG_TAIL);
    }
    break;

  case FL_MSG_ID_READ_TEMP_AND_HUM:
    if (g_app.proto_mgr.parser_handle.arg_count == 1)
    {
      fl_sensor_t* sensor = (fl_sensor_t*)&(proto_mgr->parser_handle.payload);

      if (sensor->sensor_num == FW_APP_DHT22_MIN_NUM)
      {
        ret = fl_dht22_get_readings(&g_app.dht22[0].dht22_handle);
        if (ret == FL_OK)
        {
          uint16_t temperature = 0;
          uint16_t humidity = 0;

          fl_dht22_decode_readings(&g_app.dht22[0].dht22_handle);
          temperature = fl_dht22_get_temperature(&g_app.dht22[0].dht22_handle);
          humidity = fl_dht22_get_humidity(&g_app.dht22[0].dht22_handle);

          g_app.proto_mgr.out_length = sprintf((char*)g_app.proto_mgr.out_buf, "%s %ld,%d,%d,%d.%d,%d.%d%c",
              fl_txt_msg_get_message_name(g_app.proto_mgr.parser_handle.msg_id),
              g_app.proto_mgr.parser_handle.device_id,
              FL_OK,
              sensor->sensor_num,
              (temperature & 0xffff) / 10,
              (temperature & 0x7fff) % 10,
              (humidity & 0xffff) / 10,
              (humidity & 0x7fff) % 10,
              FL_TXT_MSG_TAIL);

          cmd_processed = FL_TRUE;
        }
      }

      if (cmd_processed != FL_TRUE)
      {
        g_app.proto_mgr.out_length = sprintf((char*)g_app.proto_mgr.out_buf, "%s %ld,%d%c",
            fl_txt_msg_get_message_name(g_app.proto_mgr.parser_handle.msg_id),
            g_app.proto_mgr.parser_handle.device_id,
            FL_ERROR,
            FL_TXT_MSG_TAIL);
      }
    }
    break;

  case FL_MSG_ID_BOOT_MODE:
    if (g_app.proto_mgr.parser_handle.arg_count == 1)
    {
      cmd_processed = FL_TRUE;
      g_app.proto_mgr.out_length = sprintf((char*)g_app.proto_mgr.out_buf, "%s %ld,%d%c",
                  fl_txt_msg_get_message_name(g_app.proto_mgr.parser_handle.msg_id),
                  g_app.proto_mgr.parser_handle.device_id,
                  FL_OK,
                  FL_TXT_MSG_TAIL);
    }

    if (cmd_processed != FL_TRUE)
    {
      g_app.proto_mgr.out_length = sprintf((char*)g_app.proto_mgr.out_buf, "%s %ld,%d%c",
          fl_txt_msg_get_message_name(g_app.proto_mgr.parser_handle.msg_id),
          g_app.proto_mgr.parser_handle.device_id,
          FL_ERROR,
          FL_TXT_MSG_TAIL);
    }
    break;

  case FL_MSG_ID_RESET:
      g_app.proto_mgr.out_length = sprintf((char*)g_app.proto_mgr.out_buf, "%s %ld,%d%c",
                  fl_txt_msg_get_message_name(g_app.proto_mgr.parser_handle.msg_id),
                  g_app.proto_mgr.parser_handle.device_id,
                  FL_OK,
                  FL_TXT_MSG_TAIL);
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
  fl_bin_msg_parser_t*    bin_parser = (fl_bin_msg_parser_t*)parser_handle;
  fw_app_proto_manager_t* proto_mgr = &((fw_app_t*)context)->proto_mgr;
  fl_bin_msg_header_t*    header = (fl_bin_msg_header_t*)&bin_parser->buf[1];
  fl_bool_t               cmd_processed = FL_FALSE;
  fl_bin_msg_full_t*      tx_msg_full = (fl_bin_msg_full_t*)proto_mgr->out_buf;
  fl_bin_msg_full_t*      rx_msg_full = (fl_bin_msg_full_t*)bin_parser->buf;

  // Ignore the parsed message.
  if (header->device_id != ((fw_app_t*)context)->device_id)
  {
    return;
  }

  tx_msg_full->header.device_id = header->device_id;
  tx_msg_full->header.message_id = header->message_id;
  tx_msg_full->header.flag1.sequence_num = header->flag1.sequence_num;
  tx_msg_full->header.flag1.return_expected = FL_FALSE;
  tx_msg_full->header.flag2.error = FL_OK;

  switch (header->message_id)
  {
    case FL_MSG_ID_READ_HW_VERSION:
    {
      fl_hw_ver_t* hw_ver = (fl_hw_ver_t*)&(tx_msg_full->payload);
      sprintf(hw_ver->version, "%d.%d.%d", FW_APP_HW_MAJOR, FW_APP_HW_MINOR, FW_APP_HW_REVISION);
      proto_mgr->out_length = fl_bin_msg_build_response((uint8_t*)proto_mgr->out_buf, sizeof(proto_mgr->out_buf));
      break;
    }

    case FL_MSG_ID_READ_FW_VERSION:
    {
      fl_fw_ver_t* fw_ver = (fl_fw_ver_t*)&(tx_msg_full->payload);
      sprintf(fw_ver->version, "%d.%d.%d", FW_APP_FW_MAJOR, FW_APP_FW_MINOR, FW_APP_FW_REVISION);
      proto_mgr->out_length = fl_bin_msg_build_response((uint8_t*)proto_mgr->out_buf, sizeof(proto_mgr->out_buf));
      break;
    }

    case FL_MSG_ID_WRITE_GPIO:
    {
      fl_gpo_port_value_t* gpo_port_value = (fl_gpo_port_value_t*)&(rx_msg_full->payload);

      if ((gpo_port_value->port_num >= FW_APP_DOUT_MIN_PORT_NUM) &&
          (gpo_port_value->port_num <= FW_APP_DOUT_MAX_PORT_NUM))
      {
        if ((gpo_port_value->port_value == 0) ||
            (gpo_port_value->port_value == 1))
        {
          fl_dio_port_t* port = fw_app_get_dout_port(gpo_port_value->port_num);
          if (port != NULL)
          {
            if (fl_dio_write(port, gpo_port_value->port_value) == FL_OK)
            {
              cmd_processed = FL_TRUE;
            }
          }
        }
      }

      if (cmd_processed != FL_TRUE)
      {
        tx_msg_full->header.flag2.error = FL_ERROR;
      }

      proto_mgr->out_length = fl_bin_msg_build_response((uint8_t*)proto_mgr->out_buf, sizeof(proto_mgr->out_buf));
      break;
    }

    case FL_MSG_ID_READ_GPIO:
    {
      fl_gpi_port_t* gpi_port = (fl_gpi_port_t*)&(rx_msg_full->payload);
      fl_gpi_port_value_t* gpi_port_val = (fl_gpi_port_value_t*)&(tx_msg_full->payload);

      if ((gpi_port->port_num >= FW_APP_DIN_MIN_PORT_NUM) &&
          (gpi_port->port_num <= FW_APP_DIN_MAX_PORT_NUM))
      {
        fl_dio_port_t* port = fw_app_get_din_port(gpi_port->port_num);
        fl_bool_t din_value;

        if (fl_dio_read(port, &din_value) == FL_OK)
        {
          gpi_port_val->port_num = gpi_port->port_num;
          gpi_port_val->port_value = din_value;
          cmd_processed = FL_TRUE;
        }
      }

      if (cmd_processed != FL_TRUE)
      {
        tx_msg_full->header.flag2.error = FL_ERROR;
      }

      proto_mgr->out_length = fl_bin_msg_build_response((uint8_t*)proto_mgr->out_buf, sizeof(proto_mgr->out_buf));
      break;
    }

    case FL_MSG_ID_READ_TEMPERATURE:
    {
      fl_sensor_t* sensor = (fl_sensor_t*)&(rx_msg_full->payload);

      if (sensor->sensor_num == 1)
      {
        if (fl_dht22_get_readings(&g_app.dht22[0].dht22_handle) == FL_OK)
        {
          fl_temp_sensor_read_t* sensor_read = (fl_temp_sensor_read_t*)&(tx_msg_full->payload);

          fl_dht22_decode_readings(&g_app.dht22[0].dht22_handle);

          sensor_read->sensor_num = sensor->sensor_num;
          sensor_read->temperature = ((double)fl_dht22_get_temperature(&g_app.dht22[0].dht22_handle)) / 10.0;

          cmd_processed = FL_TRUE;
        }
      }
      else if (sensor->sensor_num == 2)
      {
        fl_ds18b20_manual_convert(&g_app.ds18b20[0]);

        if (g_app.ds18b20[0].devices[0].data_valid == FL_TRUE)
        {
          fl_temp_sensor_read_t* sensor_read = (fl_temp_sensor_read_t*)&(tx_msg_full->payload);

          sensor_read->sensor_num = sensor->sensor_num;
          sensor_read->temperature = (double)g_app.ds18b20[0].devices[0].temperature;

          cmd_processed = FL_TRUE;
        }
      }

      if (cmd_processed != FL_TRUE)
      {
        tx_msg_full->header.flag2.error = FL_ERROR;
      }
      proto_mgr->out_length = fl_bin_msg_build_response((uint8_t*)proto_mgr->out_buf, sizeof(proto_mgr->out_buf));
      break;
    }

    case FL_MSG_ID_READ_HUMIDITY:
    {
      fl_sensor_t* sensor = (fl_sensor_t*)&(rx_msg_full->payload);

      if (sensor->sensor_num == FW_APP_DHT22_MIN_NUM)
      {
        if (fl_dht22_get_readings(&g_app.dht22[0].dht22_handle) == FL_OK)
        {
          fl_hum_sensor_read_t* sensor_read = (fl_hum_sensor_read_t*)&(tx_msg_full->payload);

          fl_dht22_decode_readings(&g_app.dht22[0].dht22_handle);

          sensor_read->sensor_num = sensor->sensor_num;
          sensor_read->humidity = ((double)fl_dht22_get_humidity(&g_app.dht22[0].dht22_handle)) / 10.0;

          cmd_processed = FL_TRUE;
        }
      }

      if (cmd_processed != FL_TRUE)
      {
        tx_msg_full->header.flag2.error = FL_ERROR;
      }

      proto_mgr->out_length = fl_bin_msg_build_response((uint8_t*)proto_mgr->out_buf, sizeof(proto_mgr->out_buf));
      break;
    }

    case FL_MSG_ID_READ_TEMP_AND_HUM:
    {
      fl_sensor_t* sensor = (fl_sensor_t*)&(rx_msg_full->payload);

      if (sensor->sensor_num == FW_APP_DHT22_MIN_NUM)
      {
        if (fl_dht22_get_readings(&g_app.dht22[0].dht22_handle) == FL_OK)
        {
          fl_temp_hum_sensor_read_t* sensor_read = (fl_temp_hum_sensor_read_t*)&(tx_msg_full->payload);

          fl_dht22_decode_readings(&g_app.dht22[0].dht22_handle);

          sensor_read->sensor_num = sensor->sensor_num;
          sensor_read->temperature = ((double)fl_dht22_get_temperature(&g_app.dht22[0].dht22_handle)) / 10.0;
          sensor_read->humidity = ((double)fl_dht22_get_humidity(&g_app.dht22[0].dht22_handle)) / 10.0;

          cmd_processed = FL_TRUE;
        }
      }

      if (cmd_processed != FL_TRUE)
      {
        tx_msg_full->header.flag2.error = FL_ERROR;
      }
      proto_mgr->out_length = fl_bin_msg_build_response((uint8_t*)proto_mgr->out_buf, sizeof(proto_mgr->out_buf));
      break;
    }

    case FL_MSG_ID_BOOT_MODE:
    {
      fl_boot_mode_t* bmode = (fl_boot_mode_t*)&(rx_msg_full->payload);

      if ((bmode->boot_mode == FL_BMODE_APP) ||
          (bmode->boot_mode == FL_BMODE_BOOTLOADER))
      {
        cmd_processed = FL_TRUE;
      }

      if (cmd_processed != FL_TRUE)
      {
        tx_msg_full->header.flag2.error = FL_ERROR;
      }
      proto_mgr->out_length = fl_bin_msg_build_response((uint8_t*)proto_mgr->out_buf, sizeof(proto_mgr->out_buf));
      break;
    }

    case FL_MSG_ID_RESET:
    {
      cmd_processed = FL_TRUE;
      proto_mgr->out_length = fl_bin_msg_build_response((uint8_t*)proto_mgr->out_buf, sizeof(proto_mgr->out_buf));
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

#if defined(FW_APP_PARSER_DEBUG)
static void on_parse_started(const void* parser_handle)
{
  HAL_GPIO_WritePin(DBG_OUT1_GPIO_Port, DBG_OUT1_Pin, GPIO_PIN_SET);
}

static void on_parse_ended(const void* parser_handle)
{
  HAL_GPIO_WritePin(DBG_OUT1_GPIO_Port, DBG_OUT1_Pin, GPIO_PIN_RESET);
}
#endif

void _dht22_gpio_write(const void* gpio_handle, fl_bool_t on_off)
{
  fl_stm32_gpio_handle* st_gpio_handle = (fl_stm32_gpio_handle*)gpio_handle;

  HAL_GPIO_WritePin(st_gpio_handle->hport, st_gpio_handle->pin_num, on_off);
}

fl_bool_t _dht22_gpio_read(const void* gpio_handle)
{
  fl_stm32_gpio_handle* st_gpio_handle = (fl_stm32_gpio_handle*)gpio_handle;

  return (fl_bool_t)HAL_GPIO_ReadPin(st_gpio_handle->hport, st_gpio_handle->pin_num);
}

void _dht22_delay_us(volatile uint32_t microseconds)
{
  uint32_t clk_cycle_start = __HAL_TIM_GET_COUNTER(&htim2);

  /* Go to number of cycles for system */
  // Timer2 clock : 180 MHz(APB1 timer clock)
  //                1 us = 108000000 / 1000000 = 108 clocks
  // microsecond *= (1080000000 / 1000000) -> theoretical formula
  microseconds *= 108;

  /* Delay till end */
  while ((uint32_t)(__HAL_TIM_GET_COUNTER(&htim2) - clk_cycle_start) < microseconds)
  {
    __ASM volatile ("NOP");
  }
}
