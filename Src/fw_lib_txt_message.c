#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "fw_lib_txt_message.h"
#include "fw_lib_util.h"

FW_LIB_DECLARE(uint8_t) fw_lib_txt_msg_build_command(const uint32_t device_id, const uint8_t message_id, fw_lib_msg_arg_t* args, uint8_t* packet_buf)
{
  uint8_t len = 0;

  if (packet_buf != NULL)
  {
    switch (message_id)
    {
    case FW_LIB_MSG_ID_READ_HW_VERSION:
      // RHVER device_id\n
      // ex) RHVER 1\n
      len = sprintf((char*)packet_buf, "%s %ld%c", fw_lib_txt_msg_get_message_name(message_id), device_id, FW_LIB_TXT_MSG_TAIL);
      break;

    case FW_LIB_MSG_ID_READ_FW_VERSION:
      // RFVER device_id\n
      // ex) RFVER 1\n
      len = sprintf((char*)packet_buf, "%s %ld%c", fw_lib_txt_msg_get_message_name(message_id), device_id, FW_LIB_TXT_MSG_TAIL);
      break;

    case FW_LIB_MSG_ID_READ_GPIO:
      // RGPIO device_id,gpio_num\n
      // ex) RGPIO 1,2\n
      len = sprintf((char*)packet_buf, "%s %ld,%d%c", fw_lib_txt_msg_get_message_name(message_id), device_id, args[0].value.uint8_value, FW_LIB_TXT_MSG_TAIL);
      break;

    case FW_LIB_MSG_ID_WRITE_GPIO:
      // WGPIO device_id,button_num,button_value\n
      // ex) WGPIO 1,1,1\n
      len = sprintf((char*)packet_buf, "%s %ld,%d,%d%c", fw_lib_txt_msg_get_message_name(message_id), device_id, args[0].value.uint8_value, args[1].value.uint8_value, FW_LIB_TXT_MSG_TAIL);
      break;
    }
  }
    
  return len;
}

FW_LIB_DECLARE(uint8_t) fw_lib_txt_msg_build_response(const uint32_t device_id, const uint8_t message_id, fw_lib_msg_arg_t* args, uint8_t error, uint8_t* packet_buf)
{
  uint8_t len = 0;

  if (packet_buf != NULL)
  {
    switch (message_id)
    {
    case FW_LIB_MSG_ID_READ_HW_VERSION:
      // RHVER device_id,error,version_string\n
      // ex) RHVER 1,0,1.2.3\n
      len = sprintf((char*)packet_buf, "%s %ld,%d,%s%c", fw_lib_txt_msg_get_message_name(message_id), device_id, error, args->value.string_value, FW_LIB_TXT_MSG_TAIL);
      break;

    case FW_LIB_MSG_ID_READ_FW_VERSION:
      // RFVER device_id,error,version_string\n
      // ex) RFVER 1,0,2.3.4\n
      len = sprintf((char*)packet_buf, "%s %ld,%d,%s%c", fw_lib_txt_msg_get_message_name(message_id), device_id, error, args->value.string_value, FW_LIB_TXT_MSG_TAIL);
      break;

    case FW_LIB_MSG_ID_READ_GPIO:
      // RGPIO device_id,error,gpio_num,gpio_value\n
      // ex) RGPIO 1,0,2,0\n
      len = sprintf((char*)packet_buf, "%s %ld,%d,%d,%d%c", fw_lib_txt_msg_get_message_name(message_id), device_id, error, args[0].value.uint8_value, args[1].value.uint8_value, FW_LIB_TXT_MSG_TAIL);
      break;

    case FW_LIB_MSG_ID_WRITE_GPIO:
      // WGPIO device_id,error\n
      // ex) WGPIO 1,0\n
      len = sprintf((char*)packet_buf, "%s %ld,%d%c", fw_lib_txt_msg_get_message_name(message_id), device_id, error, FW_LIB_TXT_MSG_TAIL);
      break;
    }
  }

  return len;
}

FW_LIB_DECLARE(uint8_t) fw_lib_txt_msg_build_event(const uint32_t device_id, const uint8_t message_id, fw_lib_msg_arg_t* args, uint8_t* packet_buf)
{
  uint8_t len = 0;

  if (packet_buf != NULL)
  {
    switch (message_id)
    {
    case FW_LIB_MSG_ID_BUTTON_EVENT:
      // EBTN device_id,button_num,button_value\n
      // ex) EBTN 1,1,1\n
      len = sprintf((char*)packet_buf, "%s %ld,%d,%d%c", fw_lib_txt_msg_get_message_name(message_id), device_id, args[0].value.uint8_value, args[1].value.uint8_value, FW_LIB_TXT_MSG_TAIL);
      break;
    }
  }

  return len;
}

FW_LIB_DECLARE(char*) fw_lib_txt_msg_get_message_name(const uint8_t message_id)
{
  switch (message_id)
  {
  case FW_LIB_MSG_ID_READ_HW_VERSION:
    return FW_LIB_TXT_RHVER_STR;

  case FW_LIB_MSG_ID_READ_FW_VERSION:
    return FW_LIB_TXT_RFVER_STR;

  case FW_LIB_MSG_ID_READ_GPIO:
    return FW_LIB_TXT_RGPIO_STR;

  case FW_LIB_MSG_ID_WRITE_GPIO:
    return FW_LIB_TXT_WGPIO_STR;

  case FW_LIB_MSG_ID_BUTTON_EVENT:
    return FW_LIB_TXT_EBTN_STR;
  }

  return NULL;
}

