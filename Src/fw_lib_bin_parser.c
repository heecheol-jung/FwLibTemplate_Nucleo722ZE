#include <string.h>
#include <stdlib.h>
#include "fw_lib_bin_parser.h"
#include "fw_lib_util.h"

static fw_lib_bool_t check_header_payload(fw_lib_bin_parser_t* parser_handle);

FW_LIB_DECLARE(void) fw_lib_bin_parser_init(fw_lib_bin_parser_t* parser_handle)
{
  memset(parser_handle, 0, sizeof(fw_lib_bin_parser_t));
}

FW_LIB_DECLARE(void) fw_lib_bin_parser_clear(fw_lib_bin_parser_t* parser_handle)
{
  memset(&parser_handle->buf, 0, sizeof(parser_handle->buf));
  parser_handle->buf_pos = 0;
  parser_handle->receive_state = FW_LIB_BIN_PARSER_RCV_STS_STX;
  parser_handle->count = 0;
}

FW_LIB_DECLARE(fw_lib_status_t) fw_lib_bin_parser_parse(fw_lib_bin_parser_t* parser_handle, uint8_t data, fw_lib_bin_msg_full_t* message)
{
  fw_lib_status_t ret = FW_LIB_BIN_PARSER_PARSING;

  switch (parser_handle->receive_state)
  {
  case FW_LIB_BIN_PARSER_RCV_STS_STX:
    if (data == FW_LIB_BIN_MSG_STX)
    {
      if (parser_handle->on_parse_started_callback != NULL)
      {
        parser_handle->on_parse_started_callback((const void*)parser_handle);
      }

      parser_handle->buf[parser_handle->buf_pos++] = data;
      parser_handle->receive_state = FW_LIB_BIN_PARSER_RCV_STS_DEVICE_ID;
    }
    else
    {
      ret = FW_LIB_ERROR;
    }
    break;

  case FW_LIB_BIN_PARSER_RCV_STS_DEVICE_ID:
    if (parser_handle->count < FW_LIB_BIN_MSG_DEVICE_ID_LENGTH)
    {
      parser_handle->buf[parser_handle->buf_pos++] = data;
      parser_handle->count++;
      if (parser_handle->count == FW_LIB_BIN_MSG_DEVICE_ID_LENGTH)
      {
        parser_handle->count = 0;
        parser_handle->receive_state = FW_LIB_BIN_PARSER_RCV_STS_LENGTH;
      }
    }
    else
    {
      ret = FW_LIB_ERROR;
    }
    break;

  case FW_LIB_BIN_PARSER_RCV_STS_LENGTH:
    parser_handle->buf[parser_handle->buf_pos++] = data;
    parser_handle->receive_state = FW_LIB_BIN_PARSER_RCV_STS_HDR_DATA;
    // TODO : Check length value.
    break;

  case FW_LIB_BIN_PARSER_RCV_STS_HDR_DATA:
  {
    fw_lib_bin_msg_header_t* header = (fw_lib_bin_msg_header_t*)&parser_handle->buf[1];
    if (parser_handle->count < header->length)
    {
      parser_handle->buf[parser_handle->buf_pos++] = data;
      parser_handle->count++;
      if (parser_handle->count == header->length)
      {
        if (check_header_payload(parser_handle) == FW_LIB_TRUE)
        {
          ret = FW_LIB_OK;
        }
        else
        {
          ret = FW_LIB_ERROR;
        }
      }
    }
  }
  break;

  default:
    ret = FW_LIB_ERROR;
    break;
  }

  if (ret != FW_LIB_BIN_PARSER_PARSING)
  {
    if (ret == FW_LIB_OK)
    {
      if (parser_handle->on_parse_ended_callback != NULL)
      {
        parser_handle->on_parse_ended_callback((const void*)parser_handle);
      }

      if (parser_handle->on_parsed_callback != NULL)
      {
        parser_handle->on_parsed_callback((const void*)parser_handle, parser_handle->context);
      }
      else
      {
        if (message != NULL)
        {
          memcpy(message, parser_handle->buf, parser_handle->buf_pos);
        }
      }
    }
  }

  return ret;
}

static fw_lib_bool_t check_header_payload(fw_lib_bin_parser_t* parser_handle)
{
  fw_lib_bin_msg_header_t* header = (fw_lib_bin_msg_header_t*)&parser_handle->buf[1];
  uint8_t msg_size = 0;
  uint16_t calculated_crc = 0;
  uint16_t actual_crc = 0;
  uint8_t message_type = FW_LIB_MSG_TYPE_UNKNOWN;

  if (parser_handle->buf_pos < FW_LIB_BIN_MSG_MIN_LENGTH)
  {
    return FW_LIB_FALSE;
  }

  if (parser_handle->buf[parser_handle->buf_pos - 1] != FW_LIB_BIN_MSG_ETX)
  {
    return FW_LIB_FALSE;
  }

  msg_size = sizeof(fw_lib_bin_msg_header_t);
  message_type = FW_LIB_BIT_FIELD_GET(header->flag1, FW_LIB_BIN_MSG_HDR_FLG1_MSG_TYPE_MASK, FW_LIB_BIN_MSG_HDR_FLG1_MSG_TYPE_POS);
  if (message_type == FW_LIB_MSG_TYPE_COMMAND)
  {
    if (header->message_id == FW_LIB_MSG_ID_READ_GPIO)
    {
      msg_size = sizeof(fw_bin_msg_read_gpio_cmd_t);
    }
    else if (header->message_id == FW_LIB_MSG_ID_WRITE_GPIO)
    {
      msg_size = sizeof(fw_bin_msg_write_gpio_cmd_t);
    }
    else if ((header->message_id == FW_LIB_MSG_ID_READ_TEMPERATURE) ||
             (header->message_id == FW_LIB_MSG_ID_READ_HUMIDITY) ||
             (header->message_id == FW_LIB_MSG_ID_READ_TEMP_AND_HUM))
    {
      msg_size = sizeof(fw_bin_msg_read_dht22_cmd_t);
    }
  }
  else if (message_type == FW_LIB_MSG_TYPE_RESPONSE)
  {
    if (FW_LIB_BIT_FIELD_GET(header->flag2, FW_LIB_BIN_MSG_HDR_FLG2_ERROR_MASK, FW_LIB_BIN_MSG_HDR_FLG2_ERROR_POS) == FW_LIB_OK)
    {
      if (header->message_id == FW_LIB_MSG_ID_READ_HW_VERSION)
      {
        msg_size = sizeof(fw_bin_msg_read_hw_ver_resp_t);
      }
      else if (header->message_id == FW_LIB_MSG_ID_READ_FW_VERSION)
      {
        msg_size = sizeof(fw_bin_msg_read_fw_ver_resp_t);
      }
      else if (header->message_id == FW_LIB_MSG_ID_READ_GPIO)
      {
        msg_size = sizeof(fw_bin_msg_read_gpio_resp_t);
      }
      else if ((header->message_id == FW_LIB_MSG_ID_READ_TEMPERATURE) ||
               (header->message_id == FW_LIB_MSG_ID_READ_HUMIDITY))
      {
        msg_size = sizeof(fw_bin_msg_read_dht22_resp_t);
      }
      else if (header->message_id == FW_LIB_MSG_ID_READ_TEMP_AND_HUM)
      {
        msg_size = sizeof(fw_bin_msg_read_dht22_temp_hum_resp_t);
      }
    }
  }
  else if (message_type == FW_LIB_MSG_TYPE_EVENT)
  {
    if (header->message_id == FW_LIB_MSG_ID_BUTTON_EVENT)
    {
      msg_size = sizeof(fw_bin_msg_button_evt_t);
    }
  }
  else
  {
    return FW_LIB_FALSE;
  }

  calculated_crc = fw_lib_crc_16(&parser_handle->buf[1], msg_size);
  actual_crc = FW_LIB_SWAP_2BYTES(*((uint16_t*)&parser_handle->buf[parser_handle->buf_pos - 3]));
  if (calculated_crc != actual_crc)
  {
    return FW_LIB_FALSE;
  }

  header->device_id = FW_LIB_SWAP_4BYTES(header->device_id);

  return FW_LIB_TRUE;
}
