#include <string.h>
#include <stdlib.h>
#include "fl_txt_message_parser.h"
#include "internal_util.h"

static fl_bool_t is_command_with_arguments(uint8_t msg_id);
static void clear_receive_buffer(fl_txt_msg_parser_t* parser_handle);
static fl_bool_t process_command_data(fl_txt_msg_parser_t* parser_handle);
static fl_bool_t process_response_event_data(fl_txt_msg_parser_t* parser_handle);
static fl_bool_t is_evnet_msg(fl_txt_msg_parser_t* parser_handle);

FL_DECLARE(void) fl_txt_msg_parser_init(fl_txt_msg_parser_t* parser_handle)
{
  memset(parser_handle, 0, sizeof(fl_txt_msg_parser_t));
}

FL_DECLARE(void) fl_txt_msg_parser_clear(fl_txt_msg_parser_t* parser_handle)
{
  parser_handle->buf_pos = 0;
  parser_handle->receive_state = FL_TXT_MSG_PARSER_RCV_STS_MSG_ID;
  parser_handle->msg_id = FL_MSG_ID_UNKNOWN;
  parser_handle->arg_count = 0;
  memset(&parser_handle->payload, 0, sizeof(parser_handle->payload));
  memset(parser_handle->buf, 0, sizeof(parser_handle->buf));
}

FL_DECLARE(fl_status_t) fl_txt_msg_parser_parse_command(fl_txt_msg_parser_t* parser_handle, uint8_t data, fl_txt_msg_t* msg_handle)
{
  fl_status_t ret = FL_TXT_MSG_PARSER_PARSING;

  switch (parser_handle->receive_state)
  {
  case FL_TXT_MSG_PARSER_RCV_STS_MSG_ID:
    if (is_msg_id_char(data) == FL_TRUE)
    {
      if ((parser_handle->buf_pos == 0) &&
        (parser_handle->on_parse_started_callback != NULL))
      {
        parser_handle->on_parse_started_callback((const void*)parser_handle);
      }

      parser_handle->buf[parser_handle->buf_pos++] = data;
      if (parser_handle->buf_pos > FL_TXT_MSG_ID_MAX_LEN)
      {
        // Invalid length for a message ID.
        ret = FL_ERROR;
      }
    }
    else if (data == FL_TXT_MSG_ID_DEVICE_ID_DELIMITER)
    {
      parser_handle->msg_id = fl_txt_msg_parser_get_msg_id(parser_handle->buf, parser_handle->buf_pos);
      if (parser_handle->msg_id != FL_MSG_ID_UNKNOWN)
      {
        parser_handle->receive_state = FL_TXT_MSG_PARSER_RCV_STS_DEVICE_ID;
        clear_receive_buffer(parser_handle);
      }
      else
      {
        // Unknown message ID.
        ret = FL_ERROR;
      }
    }
    else
    {
      // Invalid character for a message ID.
      ret = FL_ERROR;
    }
    break;

  case FL_TXT_MSG_PARSER_RCV_STS_DEVICE_ID:
    if (is_device_id_char(data) == FL_TRUE)
    {
      parser_handle->buf[parser_handle->buf_pos++] = data;
      if (parser_handle->buf_pos > FL_TXT_MSG_DEVICE_ID_MAX_LEN)
      {
        // Invalid length for device ID.
        ret = FL_ERROR;
      }
    }
    else if (data == FL_TXT_MSG_ARG_DELIMITER)
    {
      parser_handle->device_id = get_device_id(parser_handle->buf, parser_handle->buf_pos);
      if (is_command_with_arguments(parser_handle->msg_id) == FL_TRUE)
      {
        parser_handle->receive_state = FL_TXT_MSG_PARSER_RCV_STS_DATA;
        clear_receive_buffer(parser_handle);
      }
      else
      {
        // Invalid argument : Received command does not need arguments.
        ret = FL_ERROR;
      }
    }
    else if (is_tail(data) == FL_TRUE)
    {
      parser_handle->device_id = get_device_id(parser_handle->buf, parser_handle->buf_pos);
      parser_handle->receive_state = FL_TXT_MSG_PARSER_RCV_STS_TAIL;
      ret = FL_OK;
    }
    else
    {
      ret = FL_ERROR;
    }
    break;

  case FL_TXT_MSG_PARSER_RCV_STS_DATA:
    if (is_tail(data) != FL_TRUE)
    {
      if (data != FL_TXT_MSG_ARG_DELIMITER)
      {
        parser_handle->buf[parser_handle->buf_pos++] = data;
        if (parser_handle->buf_pos >= FL_TXT_MSG_MAX_LENGTH)
        {
          ret = FL_ERROR;
        }
      }
      else
      {
        if (process_command_data(parser_handle) == FL_TRUE)
        {
          clear_receive_buffer(parser_handle);
        }
        else
        {
          ret = FL_ERROR;
        }
      }
    }
    else
    {
      parser_handle->receive_state = FL_TXT_MSG_PARSER_RCV_STS_TAIL;
      if (process_command_data(parser_handle) == FL_TRUE)
      {
        clear_receive_buffer(parser_handle);
        ret = FL_OK;
      }
      else
      {
        ret = FL_ERROR;
      }
    }
    break;

  default:
    ret = FL_ERROR;
    break;
  }

  if (ret != FL_TXT_MSG_PARSER_PARSING)
  {
    if (ret == FL_OK)
    {
      if (parser_handle->on_parse_ended_callback != NULL)
      {
        parser_handle->on_parse_ended_callback((const void*)parser_handle);
      }

      if (parser_handle->on_parsed_callback != NULL)
      {
        parser_handle->on_parsed_callback((const void*)parser_handle, parser_handle->context);
        fl_txt_msg_parser_clear(parser_handle);
      }
      else
      {
        if (msg_handle != NULL)
        {
          msg_handle->device_id = parser_handle->device_id;
          msg_handle->msg_id = parser_handle->msg_id;
          if (parser_handle->arg_count > 0)
          {
            memcpy(&msg_handle->payload, &parser_handle->payload, sizeof(fl_fw_ver_t));
          }
          fl_txt_msg_parser_clear(parser_handle);
        }
      }
    }
  }

  return ret;
}

FL_DECLARE(fl_status_t) fl_txt_msg_parser_parse_response_event(fl_txt_msg_parser_t* parser_handle, uint8_t data, fl_txt_msg_t* msg_handle)
{
  fl_status_t ret = FL_TXT_MSG_PARSER_PARSING;

  switch (parser_handle->receive_state)
  {
  case FL_TXT_MSG_PARSER_RCV_STS_MSG_ID:
    if (is_msg_id_char(data) == FL_TRUE)
    {
      parser_handle->buf[parser_handle->buf_pos++] = data;
      if (parser_handle->buf_pos > FL_TXT_MSG_ID_MAX_LEN)
      {
        // Invalid length for a message ID.
        ret = FL_ERROR;
      }
    }
    else if (data == FL_TXT_MSG_ID_DEVICE_ID_DELIMITER)
    {
      parser_handle->msg_id = fl_txt_msg_parser_get_msg_id(parser_handle->buf, parser_handle->buf_pos);
      if (parser_handle->msg_id != FL_MSG_ID_UNKNOWN)
      {
        parser_handle->receive_state = FL_TXT_MSG_PARSER_RCV_STS_DEVICE_ID;
        clear_receive_buffer(parser_handle);
      }
      else
      {
        // Unknown message ID.
        ret = FL_ERROR;
      }
    }
    else
    {
      // Invalid character for a message ID.
      ret = FL_ERROR;
    }
    break;

  case FL_TXT_MSG_PARSER_RCV_STS_DEVICE_ID:
    if (is_device_id_char(data) == FL_TRUE)
    {
      parser_handle->buf[parser_handle->buf_pos++] = data;
      if (parser_handle->buf_pos > FL_TXT_MSG_DEVICE_ID_MAX_LEN)
      {
        // Invalid length for device ID.
        ret = FL_ERROR;
      }
    }
    else if (data == FL_TXT_MSG_ARG_DELIMITER)
    {
      parser_handle->device_id = get_device_id(parser_handle->buf, parser_handle->buf_pos);
      if (is_evnet_msg(parser_handle) == FL_TRUE)
      {
        parser_handle->receive_state = FL_TXT_MSG_PARSER_RCV_STS_DATA;
      }
      else
      {
        parser_handle->receive_state = FL_TXT_MSG_PARSER_RCV_STS_ERROR;
      }
      clear_receive_buffer(parser_handle);
    }
    else
    {
      ret = FL_ERROR;
    }
    break;

  case FL_TXT_MSG_PARSER_RCV_STS_ERROR:
    if (is_tail(data) == FL_TRUE)
    {
      parser_handle->error = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->receive_state = FL_TXT_MSG_PARSER_RCV_STS_TAIL;
      clear_receive_buffer(parser_handle);
      ret = FL_OK;
    }
    else if (data != FL_TXT_MSG_ARG_DELIMITER)
    {
      parser_handle->buf[parser_handle->buf_pos++] = data;
      if (parser_handle->buf_pos > FL_TXT_MSG_ERROR_MAX_LEN)
      {
        // Invalid length for error code.
        ret = FL_ERROR;
      }
    }
    else
    {
      parser_handle->error = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->receive_state = FL_TXT_MSG_PARSER_RCV_STS_DATA;
      clear_receive_buffer(parser_handle);
    }
    break;

  case FL_TXT_MSG_PARSER_RCV_STS_DATA:
    if (is_tail(data) != FL_TRUE)
    {
      if (data != FL_TXT_MSG_ARG_DELIMITER)
      {
        parser_handle->buf[parser_handle->buf_pos++] = data;
        if (parser_handle->buf_pos >= FL_TXT_MSG_MAX_LENGTH)
        {
          ret = FL_ERROR;
        }
      }
      else
      {
        if (process_response_event_data(parser_handle) == FL_TRUE)
        {
          clear_receive_buffer(parser_handle);
        }
        else
        {
          ret = FL_ERROR;
        }
      }
    }
    else
    {
      parser_handle->receive_state = FL_TXT_MSG_PARSER_RCV_STS_TAIL;
      if (process_response_event_data(parser_handle) == FL_TRUE)
      {
        clear_receive_buffer(parser_handle);
        ret = FL_OK;
      }
      else
      {
        ret = FL_ERROR;
      }
    }
    break;

  default:
    ret = FL_ERROR;
    break;
  }

  if (ret != FL_TXT_MSG_PARSER_PARSING)
  {
    if (ret == FL_OK)
    {
      if (parser_handle->on_parsed_callback != NULL)
      {
        parser_handle->on_parsed_callback((const void*)parser_handle, parser_handle->context);
      }
      else
      {
        if (msg_handle != NULL)
        {
          msg_handle->device_id = parser_handle->device_id;
          msg_handle->msg_id = parser_handle->msg_id;
          msg_handle->error = parser_handle->error;
          if (parser_handle->arg_count > 0)
          {
            memcpy(&msg_handle->payload, &parser_handle->payload, sizeof(fl_fw_ver_t));
          }
        }
      }
    }

    //fl_txt_msg_parser_clear(parser_handle);
  }

  return ret;
}

FL_DECLARE(uint8_t) fl_txt_msg_parser_get_msg_id(uint8_t* buf, uint8_t buf_size)
{
  if (buf_size == 4)
  {
    if (strcmp(FL_TXT_EBTN_STR, (const char*)buf) == 0)
    {
      return FL_MSG_ID_BUTTON_EVENT;
    }
    else if (strcmp(FL_TXT_RHUM_STR, (const char*)buf) == 0)
    {
      return FL_MSG_ID_READ_HUMIDITY;
    }
    else if (strcmp(FL_TXT_RTAH_STR, (const char*)buf) == 0)
    {
      return FL_MSG_ID_READ_TEMP_AND_HUM;
    }
  }
  else if (buf_size == 5)
  {
    if (strcmp(FL_TXT_RHVER_STR, (const char*)buf) == 0)
    {
      return FL_MSG_ID_READ_HW_VERSION;
    }
    else if (strcmp(FL_TXT_RFVER_STR, (const char*)buf) == 0)
    {
      return FL_MSG_ID_READ_FW_VERSION;
    }
    else if (strcmp(FL_TXT_RGPIO_STR, (const char*)buf) == 0)
    {
      return FL_MSG_ID_READ_GPIO;
    }
    else if (strcmp(FL_TXT_WGPIO_STR, (const char*)buf) == 0)
    {
      return FL_MSG_ID_WRITE_GPIO;
    }
    else if (strcmp(FL_TXT_RTEMP_STR, (const char*)buf) == 0)
    {
      return FL_MSG_ID_READ_TEMPERATURE;
    }
    else if (strcmp(FL_TXT_BMODE_STR, (const char*)buf) == 0)
    {
      return FL_MSG_ID_BOOT_MODE;
    }
    else if (strcmp(FL_TXT_RESET_STR, (const char*)buf) == 0)
    {
      return FL_MSG_ID_RESET;
    }
  }

  return FL_MSG_ID_UNKNOWN;
}

static fl_bool_t is_command_with_arguments(uint8_t msg_id)
{
  switch (msg_id)
  {
  case FL_MSG_ID_READ_GPIO:
  case FL_MSG_ID_WRITE_GPIO:
  case FL_MSG_ID_READ_TEMPERATURE:
  case FL_MSG_ID_READ_HUMIDITY:
  case FL_MSG_ID_READ_TEMP_AND_HUM:
  case FL_MSG_ID_BOOT_MODE:
    return FL_TRUE;
  }
  return FL_FALSE;
}

static void clear_receive_buffer(fl_txt_msg_parser_t* parser_handle)
{
  parser_handle->buf_pos = 0;
  memset(parser_handle->buf, 0, sizeof(parser_handle->buf));
}

static fl_bool_t process_command_data(fl_txt_msg_parser_t* parser_handle)
{
  fl_bool_t ret = FL_FALSE;

  if (parser_handle->arg_count >= FL_TXT_MSG_MAX_ARG_COUNT)
  {
    return ret;
  }

  if (parser_handle->msg_id == FL_MSG_ID_READ_GPIO)
  {
    if (parser_handle->arg_count < 1)
    {
      fl_gpi_port_t* gpi_port = (fl_gpi_port_t*)&parser_handle->payload;

      gpi_port->port_num = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
  }
  else if (parser_handle->msg_id == FL_MSG_ID_WRITE_GPIO)
  {
    fl_gpo_port_value_t* gpo_port = (fl_gpo_port_value_t*)&parser_handle->payload;
    if (parser_handle->arg_count == 0)
    {
      gpo_port->port_num = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
    else if (parser_handle->arg_count == 1)
    {
      gpo_port->port_value = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
  }
  else if ((parser_handle->msg_id == FL_MSG_ID_READ_TEMPERATURE) ||
           (parser_handle->msg_id == FL_MSG_ID_READ_HUMIDITY) ||
           (parser_handle->msg_id == FL_MSG_ID_READ_TEMP_AND_HUM))
  {
    fl_sensor_t* sensor = (fl_sensor_t*)&parser_handle->payload;
    if (parser_handle->arg_count == 0)
    {
      sensor->sensor_num = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
  }
  else if (parser_handle->msg_id == FL_MSG_ID_BOOT_MODE)
  {
    fl_boot_mode_t* bmode = (fl_boot_mode_t*)&parser_handle->payload;
    if (parser_handle->arg_count == 0)
    {
      bmode->boot_mode = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
  }

  return ret;
}

static fl_bool_t process_response_event_data(fl_txt_msg_parser_t* parser_handle)
{
  fl_bool_t ret = FL_FALSE;

  if (parser_handle->arg_count >= FL_TXT_MSG_MAX_ARG_COUNT)
  {
    return ret;
  }

  if (parser_handle->msg_id == FL_MSG_ID_READ_GPIO)
  {
    fl_gpi_port_value_t* gpi_port = (fl_gpi_port_value_t*)&parser_handle->payload;
    if (parser_handle->arg_count == 0)
    {
      gpi_port->port_num = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
    else if (parser_handle->arg_count == 1)
    {
      gpi_port->port_value = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
  }
  else if (parser_handle->msg_id == FL_MSG_ID_BUTTON_EVENT)
  {
    fl_btn_status_t* btn_sts = (fl_btn_status_t*)&parser_handle->payload;
    if (parser_handle->arg_count == 0)
    {
      btn_sts->button_num = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
    else if (parser_handle->arg_count == 1)
    {
      btn_sts->button_value = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
  }
  else if (parser_handle->msg_id == FL_MSG_ID_READ_HW_VERSION)
  {
    if (parser_handle->arg_count == 0)
    {
      fl_hw_ver_t* hw_ver = (fl_hw_ver_t*)&parser_handle->payload;
      memcpy(hw_ver->version, parser_handle->buf, parser_handle->buf_pos);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
  }
  else if (parser_handle->msg_id == FL_MSG_ID_READ_FW_VERSION)
  {
    if (parser_handle->arg_count == 0)
    {
      fl_fw_ver_t* fw_ver = (fl_fw_ver_t*)&parser_handle->payload;
      memcpy(fw_ver->version, parser_handle->buf, parser_handle->buf_pos);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
  }
  else if (parser_handle->msg_id == FL_MSG_ID_READ_TEMPERATURE)
  {
    fl_temp_sensor_read_t* temp_sensor = (fl_temp_sensor_read_t*)&parser_handle->payload;
    if (parser_handle->arg_count == 0)
    {
      temp_sensor->sensor_num = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
    else if (parser_handle->arg_count == 1)
    {
      temp_sensor->temperature = atof((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
  }
  else if (parser_handle->msg_id == FL_MSG_ID_READ_HUMIDITY)
  {
    fl_hum_sensor_read_t* hum_sensor = (fl_hum_sensor_read_t*)&parser_handle->payload;
    if (parser_handle->arg_count == 0)
    {
      hum_sensor->sensor_num = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
    else if (parser_handle->arg_count == 1)
    {
      hum_sensor->humidity = atof((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
  }
  else if (parser_handle->msg_id == FL_MSG_ID_READ_TEMP_AND_HUM)
  {
    fl_temp_hum_sensor_read_t* temp_hum_sensor = (fl_temp_hum_sensor_read_t*)&parser_handle->payload;
    if (parser_handle->arg_count == 0)
    {
      temp_hum_sensor->sensor_num = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
    else if (parser_handle->arg_count == 1)
    {
      temp_hum_sensor->temperature = atof((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
    else if (parser_handle->arg_count == 2)
    {
      temp_hum_sensor->humidity = atof((const char*)parser_handle->buf);
      parser_handle->arg_count++;
      ret = FL_TRUE;
    }
  }

  return ret;
}

static fl_bool_t is_evnet_msg(fl_txt_msg_parser_t* parser_handle)
{
  if (parser_handle->msg_id == FL_MSG_ID_BUTTON_EVENT)
  {
    return FL_TRUE;
  }
  else
  {
    return FL_FALSE;
  }
}
