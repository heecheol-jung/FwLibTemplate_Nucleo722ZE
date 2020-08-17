#include <string.h>
#include <stdlib.h>
#include "fw_lib_txt_parser.h"
#include "fw_lib_util.h"

static fw_lib_bool_t is_msg_id_char(uint8_t data);
static fw_lib_bool_t is_device_id_char(uint8_t data);
static uint8_t get_device_id(uint8_t* buf, uint8_t buf_size);
static fw_lib_bool_t is_tail(uint8_t data);
static fw_lib_bool_t is_command_with_arguments(uint8_t msg_id);
static void clear_receive_buffer(fw_lib_txt_parser_t* parser_handle);
static fw_lib_bool_t process_command_data(fw_lib_txt_parser_t* parser_handle);
static fw_lib_bool_t process_response_event_data(fw_lib_txt_parser_t* parser_handle);

FW_LIB_DECLARE(void) fw_lib_txt_parser_init(fw_lib_txt_parser_t* parser_handle)
{
  memset(parser_handle, 0, sizeof(fw_lib_txt_parser_t));
}

FW_LIB_DECLARE(void) fw_lib_txt_parser_clear(fw_lib_txt_parser_t* parser_handle)
{
  parser_handle->buf_pos = 0;
  parser_handle->receive_state = FW_LIB_TXT_PARSER_RCV_STS_MSD_ID;
  parser_handle->msg_id = FW_LIB_MSG_ID_UNKNOWN;
  parser_handle->arg_count = 0;
}

FW_LIB_DECLARE(fw_lib_status_t) fw_lib_txt_parser_parse_command(fw_lib_txt_parser_t* parser_handle, uint8_t data, fw_lib_txt_msg_t* msg_handle)
{
  fw_lib_status_t ret = FW_LIB_TXT_PARSER_PARSING;

  switch (parser_handle->receive_state)
  {
  case FW_LIB_TXT_PARSER_RCV_STS_MSD_ID:
    if (is_msg_id_char(data) == FW_LIB_TRUE)
    {
      parser_handle->buf[parser_handle->buf_pos++] = data;
      if (parser_handle->buf_pos > FW_LIB_TXT_MSG_ID_MAX_LEN)
      {
        // Invalid length for a message ID.
        ret = FW_LIB_ERROR;
      }
    }
    else if (data == FW_LIB_TXT_MSG_ID_DEVICE_ID_DELIMITER)
    {
      parser_handle->msg_id = fw_lib_txt_parser_get_msg_id(parser_handle->buf, parser_handle->buf_pos);
      if (parser_handle->msg_id != FW_LIB_MSG_ID_UNKNOWN)
      {
        parser_handle->receive_state = FW_LIB_TXT_PARSER_RCV_STS_DEVICE_ID;
        clear_receive_buffer(parser_handle);
      }
      else
      {
        // Unknown message ID.
        ret = FW_LIB_ERROR;
      }
    }
    else
    {
      // Invalid character for a message ID.
      ret = FW_LIB_ERROR;
    }
    break;

  case FW_LIB_TXT_PARSER_RCV_STS_DEVICE_ID:
    if (is_device_id_char(data) == FW_LIB_TRUE)
    {
      parser_handle->buf[parser_handle->buf_pos++] = data;
      if (parser_handle->buf_pos > FW_LIB_TXT_DEVICE_ID_MAX_LEN)
      {
        // Invalid length for device ID.
        ret = FW_LIB_ERROR;
      }
    }
    else if (data == FW_LIB_TXT_MSG_ARG_DELIMITER)
    {
      parser_handle->device_id = get_device_id(parser_handle->buf, parser_handle->buf_pos);
      if (is_command_with_arguments(parser_handle->msg_id) == FW_LIB_TRUE)
      {
        parser_handle->receive_state = FW_LIB_TXT_PARSER_RCV_STS_DATA;
        clear_receive_buffer(parser_handle);
      }
      else
      {
        // Invalid argument : Received command does not need arguments.
        ret = FW_LIB_ERROR;
      }
    }
    else if (is_tail(data) == FW_LIB_TRUE)
    {
      parser_handle->device_id = get_device_id(parser_handle->buf, parser_handle->buf_pos);
      parser_handle->receive_state = FW_LIB_TXT_PARSER_RCV_STS_TAIL;
      ret = FW_LIB_OK;
    }
    else
    {
      ret = FW_LIB_ERROR;
    }
    break;

  case FW_LIB_TXT_PARSER_RCV_STS_DATA:
    if (is_tail(data) != FW_LIB_TRUE)
    {
      if (data != FW_LIB_TXT_MSG_ARG_DELIMITER)
      {
        parser_handle->buf[parser_handle->buf_pos++] = data;
        if (parser_handle->buf_pos >= FW_LIB_TXT_MSG_MAX_LENGTH)
        {
          ret = FW_LIB_ERROR;
        }
      }
      else
      {
        if (process_command_data(parser_handle) == FW_LIB_TRUE)
        {
          clear_receive_buffer(parser_handle);
        }
        else
        {
          ret = FW_LIB_ERROR;
        }
      }
    }
    else
    {
      parser_handle->receive_state = FW_LIB_TXT_PARSER_RCV_STS_TAIL;
      if (process_command_data(parser_handle) == FW_LIB_TRUE)
      {
        clear_receive_buffer(parser_handle);
        ret = FW_LIB_OK;
      }
      else
      {
        ret = FW_LIB_ERROR;
      }
    }
    break;

  default:
    ret = FW_LIB_ERROR;
    break;
  }

  if (ret != FW_LIB_TXT_PARSER_PARSING)
  {
    if (ret == FW_LIB_OK)
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
          msg_handle->arg_count = parser_handle->arg_count;
          if (parser_handle->arg_count > 0)
          {
            memcpy(msg_handle->args, parser_handle->args, sizeof(fw_lib_msg_arg_t) * parser_handle->arg_count);
          }
        }
      }
    }
  }

  return ret;
}

FW_LIB_DECLARE(fw_lib_status_t) fw_lib_txt_parser_parse_response_event(fw_lib_txt_parser_t* parser_handle, uint8_t data, fw_lib_txt_msg_t* msg_handle)
{
  fw_lib_status_t ret = FW_LIB_TXT_PARSER_PARSING;

  switch (parser_handle->receive_state)
  {
  case FW_LIB_TXT_PARSER_RCV_STS_MSD_ID:
    if (is_msg_id_char(data) == FW_LIB_TRUE)
    {
      parser_handle->buf[parser_handle->buf_pos++] = data;
      if (parser_handle->buf_pos > FW_LIB_TXT_MSG_ID_MAX_LEN)
      {
        // Invalid length for a message ID.
        ret = FW_LIB_ERROR;
      }
    }
    else if (data == FW_LIB_TXT_MSG_ID_DEVICE_ID_DELIMITER)
    {
      parser_handle->msg_id = fw_lib_txt_parser_get_msg_id(parser_handle->buf, parser_handle->buf_pos);
      if (parser_handle->msg_id != FW_LIB_MSG_ID_UNKNOWN)
      {
        parser_handle->receive_state = FW_LIB_TXT_PARSER_RCV_STS_DEVICE_ID;
        clear_receive_buffer(parser_handle);
      }
      else
      {
        // Unknown message ID.
        ret = FW_LIB_ERROR;
      }
    }
    else
    {
      // Invalid character for a message ID.
      ret = FW_LIB_ERROR;
    }
    break;

  case FW_LIB_TXT_PARSER_RCV_STS_DEVICE_ID:
    if (is_device_id_char(data) == FW_LIB_TRUE)
    {
      parser_handle->buf[parser_handle->buf_pos++] = data;
      if (parser_handle->buf_pos > FW_LIB_TXT_DEVICE_ID_MAX_LEN)
      {
        // Invalid length for device ID.
        ret = FW_LIB_ERROR;
      }
    }
    else if (data == FW_LIB_TXT_MSG_ARG_DELIMITER)
    {
      parser_handle->device_id = get_device_id(parser_handle->buf, parser_handle->buf_pos);
      parser_handle->receive_state = FW_LIB_TXT_PARSER_RCV_STS_DATA;
      clear_receive_buffer(parser_handle);
    }
    else
    {
      ret = FW_LIB_ERROR;
    }
    break;

  case FW_LIB_TXT_PARSER_RCV_STS_DATA:
    if (is_tail(data) != FW_LIB_TRUE)
    {
      if (data != FW_LIB_TXT_MSG_ARG_DELIMITER)
      {
        parser_handle->buf[parser_handle->buf_pos++] = data;
        if (parser_handle->buf_pos >= FW_LIB_TXT_MSG_MAX_LENGTH)
        {
          ret = FW_LIB_ERROR;
        }
      }
      else
      {
        if (process_response_event_data(parser_handle) == FW_LIB_TRUE)
        {
          clear_receive_buffer(parser_handle);
        }
        else
        {
          ret = FW_LIB_ERROR;
        }
      }
    }
    else
    {
      parser_handle->receive_state = FW_LIB_TXT_PARSER_RCV_STS_TAIL;
      if (process_response_event_data(parser_handle) == FW_LIB_TRUE)
      {
        clear_receive_buffer(parser_handle);
        ret = FW_LIB_OK;
      }
      else
      {
        ret = FW_LIB_ERROR;
      }
    }
    break;

  default:
    ret = FW_LIB_ERROR;
    break;
  }

  if (ret != FW_LIB_TXT_PARSER_PARSING)
  {
    if (ret == FW_LIB_OK)
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
          msg_handle->arg_count = parser_handle->arg_count;
          if (parser_handle->arg_count > 0)
          {
            memcpy(msg_handle->args, parser_handle->args, sizeof(fw_lib_msg_arg_t) * parser_handle->arg_count);
          }
        }
      }
    }

    fw_lib_txt_parser_clear(parser_handle);
  }

  return ret;
}

FW_LIB_DECLARE(uint8_t) fw_lib_txt_parser_get_msg_id(uint8_t* buf, uint8_t buf_size)
{
  if (buf_size == 4)
  {
    if (strcmp(FW_LIB_TXT_EBTN_STR, (const char*)buf) == 0)
    {
      return FW_LIB_MSG_ID_BUTTON_EVENT;
    }
  }
  else if (buf_size == 5)
  {
    if (strcmp(FW_LIB_TXT_RHVER_STR, (const char*)buf) == 0)
    {
      return FW_LIB_MSG_ID_READ_HW_VERSION;
    }
    else if (strcmp(FW_LIB_TXT_RFVER_STR, (const char*)buf) == 0)
    {
      return FW_LIB_MSG_ID_READ_FW_VERSION;
    }
    else if (strcmp(FW_LIB_TXT_RGPIO_STR, (const char*)buf) == 0)
    {
      return FW_LIB_MSG_ID_READ_GPIO;
    }
    else if (strcmp(FW_LIB_TXT_WGPIO_STR, (const char*)buf) == 0)
    {
      return FW_LIB_MSG_ID_WRITE_GPIO;
    }
  }

  return FW_LIB_MSG_ID_UNKNOWN;
}

static uint8_t get_device_id(uint8_t* buf, uint8_t buf_size)
{
  return (uint8_t)atoi((const char*)buf);
}

static fw_lib_bool_t is_msg_id_char(uint8_t data)
{
  if ((data >= FW_LIB_TXT_MSG_ID_MIN_CHAR) &&
      (data <= FW_LIB_TXT_MSG_ID_MAX_CHAR))
  {
    return FW_LIB_TRUE;
  }
  else
  {
    return FW_LIB_FALSE;
  }
}

static fw_lib_bool_t is_device_id_char(uint8_t data)
{
  if ((data >= FW_LIB_TXT_DEVICE_ID_MIN_CHAR) &&
      (data <= FW_LIB_TXT_DEVICE_ID_MAX_CHAR))
  {
    return FW_LIB_TRUE;
  }
  else
  {
    return FW_LIB_FALSE;
  }
}

static fw_lib_bool_t is_tail(uint8_t data)
{
  if (data == FW_LIB_TXT_MSG_TAIL)
  {
    return FW_LIB_TRUE;
  }
  else
  {
    return FW_LIB_FALSE;
  }
}

static fw_lib_bool_t is_command_with_arguments(uint8_t msg_id)
{
  switch (msg_id)
  {
  case FW_LIB_MSG_ID_READ_GPIO:
  case FW_LIB_MSG_ID_WRITE_GPIO:
    return FW_LIB_TRUE;
  }
  return FW_LIB_FALSE;
}

static void clear_receive_buffer(fw_lib_txt_parser_t* parser_handle)
{
  parser_handle->buf_pos = 0;
  memset(parser_handle->buf, 0, sizeof(parser_handle->buf));
}

static fw_lib_bool_t process_command_data(fw_lib_txt_parser_t* parser_handle)
{
  fw_lib_bool_t ret = FW_LIB_FALSE;

  if (parser_handle->arg_count >= FW_LIB_TXT_MSG_MAX_ARG_COUNT)
  {
    return ret;
  }

  if (parser_handle->msg_id == FW_LIB_MSG_ID_READ_GPIO)
  {
    if (parser_handle->arg_count < 1)
    {
      parser_handle->args[parser_handle->arg_count].type = FW_LIB_ARG_TYPE_UINT8;
      parser_handle->args[parser_handle->arg_count].value.uint8_value = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;

      ret = FW_LIB_TRUE;
    }
  }
  else if (parser_handle->msg_id == FW_LIB_MSG_ID_WRITE_GPIO)
  {
    if (parser_handle->arg_count < 2)
    {
      parser_handle->args[parser_handle->arg_count].type = FW_LIB_ARG_TYPE_UINT8;
      parser_handle->args[parser_handle->arg_count].value.uint8_value = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;

      ret = FW_LIB_TRUE;
    }
  }

  return ret;
}

static fw_lib_bool_t process_response_event_data(fw_lib_txt_parser_t* parser_handle)
{
  fw_lib_bool_t ret = FW_LIB_FALSE;

  if (parser_handle->arg_count >= FW_LIB_TXT_MSG_MAX_ARG_COUNT)
  {
    return ret;
  }

  if (parser_handle->msg_id == FW_LIB_MSG_ID_READ_GPIO)
  {
    if (parser_handle->arg_count < 3)
    {
      parser_handle->args[parser_handle->arg_count].type = FW_LIB_ARG_TYPE_UINT8;
      parser_handle->args[parser_handle->arg_count].value.uint8_value = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;

      ret = FW_LIB_TRUE;
    }
  }
  else if (parser_handle->msg_id == FW_LIB_MSG_ID_WRITE_GPIO)
  {
    if (parser_handle->arg_count < 1)
    {
      parser_handle->args[parser_handle->arg_count].type = FW_LIB_ARG_TYPE_UINT8;
      parser_handle->args[parser_handle->arg_count].value.uint8_value = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;

      ret = FW_LIB_TRUE;
    }
  }
  else if (parser_handle->msg_id == FW_LIB_MSG_ID_BUTTON_EVENT)
  {
    if (parser_handle->arg_count < 2)
    {
      parser_handle->args[parser_handle->arg_count].type = FW_LIB_ARG_TYPE_UINT8;
      parser_handle->args[parser_handle->arg_count].value.uint8_value = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;

      ret = FW_LIB_TRUE;
    }
  }
  else if ((parser_handle->msg_id == FW_LIB_MSG_ID_READ_HW_VERSION) ||
           (parser_handle->msg_id == FW_LIB_MSG_ID_READ_FW_VERSION))
  {
    if (parser_handle->arg_count == 0)
    {
      parser_handle->args[parser_handle->arg_count].type = FW_LIB_ARG_TYPE_UINT8;
      parser_handle->args[parser_handle->arg_count].value.uint8_value = (uint8_t)atoi((const char*)parser_handle->buf);
      parser_handle->arg_count++;

      ret = FW_LIB_TRUE;
    }
    else if (parser_handle->arg_count == 1)
    {
      parser_handle->args[parser_handle->arg_count].type = FW_LIB_ARG_TYPE_STRING;
      memcpy(parser_handle->args[parser_handle->arg_count].value.string_value, parser_handle->buf, parser_handle->buf_pos);
      parser_handle->arg_count++;

      ret = FW_LIB_TRUE;
    }
  }

  return ret;
}
