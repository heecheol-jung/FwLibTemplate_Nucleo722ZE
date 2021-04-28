#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "fl_txt_message.h"

FL_DECLARE(uint8_t) fl_txt_msg_build_command(
  const uint32_t device_id,
  const uint8_t message_id,
  void* arg_buf,
  uint32_t arg_buf_len,
  uint8_t* packet_buf,
  uint32_t packet_buf_len)
{
  uint32_t len = 0;

  if ((packet_buf == NULL) ||
      (packet_buf_len == 0))
  {
    return len;
  }

  if (arg_buf != NULL)
  {
    if (arg_buf_len == 0)
    {
      return len;
    }
    else if (arg_buf_len > sizeof(fl_fw_ver_t))
    {
      return len;
    }
  }
  else
  {
    if (arg_buf != 0)
    {
      return len;
    }
  }

  switch (message_id)
  {
  case FL_MSG_ID_READ_HW_VERSION:
  {
    // RHVER device_id\n
    // ex) RHVER 1\n
    len = sprintf((char*)packet_buf, "%s %ld%c", fl_txt_msg_get_message_name(message_id), device_id, FL_TXT_MSG_TAIL);
    break;
  }

  case FL_MSG_ID_READ_FW_VERSION:
  {
    // RFVER device_id\n
    // ex) RFVER 1\n
    len = sprintf((char*)packet_buf, "%s %ld%c", fl_txt_msg_get_message_name(message_id), device_id, FL_TXT_MSG_TAIL);
    break;
  }

  case FL_MSG_ID_READ_GPIO:
  {
    // RGPIO device_id,gpio_num\n
    // ex) RGPIO 1,2\n
    fl_gpi_port_t* gpi_port = (fl_gpi_port_t*)arg_buf;
    if (arg_buf_len == sizeof(fl_gpi_port_t))
    {
      len = sprintf((char*)packet_buf, "%s %ld,%d%c", fl_txt_msg_get_message_name(message_id), device_id, gpi_port->port_num, FL_TXT_MSG_TAIL);
    }
    break;
  }

  case FL_MSG_ID_WRITE_GPIO:
  {
    // WGPIO device_id,button_num,button_value\n
    // ex) WGPIO 1,1,1\n
    fl_gpo_port_value_t* gpo_port = (fl_gpo_port_value_t*)arg_buf;
    if (arg_buf_len == sizeof(fl_gpo_port_value_t))
    {
      len = sprintf((char*)packet_buf, "%s %ld,%d,%d%c", fl_txt_msg_get_message_name(message_id), device_id, gpo_port->port_num, gpo_port->port_value, FL_TXT_MSG_TAIL);
    }
    break;
  }

  case FL_MSG_ID_READ_TEMPERATURE:
  {
    // RTEMP device_id,sensor_num\n
    // ex) RTEMP 1,1\n
    fl_sensor_t* sensor = (fl_sensor_t*)arg_buf;
    if (arg_buf_len == sizeof(fl_sensor_t))
    {
      len = sprintf((char*)packet_buf, "%s %ld,%d%c", fl_txt_msg_get_message_name(message_id), device_id, sensor->sensor_num, FL_TXT_MSG_TAIL);
    }
    break;
  }

  case FL_MSG_ID_READ_HUMIDITY:
  {
    // RHUM device_id,sensor_num\n
    // ex) RHUM 1,1\n
    fl_sensor_t* sensor = (fl_sensor_t*)arg_buf;
    if (arg_buf_len == sizeof(fl_sensor_t))
    {
      len = sprintf((char*)packet_buf, "%s %ld,%d%c", fl_txt_msg_get_message_name(message_id), device_id, sensor->sensor_num, FL_TXT_MSG_TAIL);
    }
    break;
  }

  case FL_MSG_ID_READ_TEMP_AND_HUM:
  {
    // RTAH device_id,sensor_num\n
    // ex) RHUM 1,1\n
    fl_sensor_t* sensor = (fl_sensor_t*)arg_buf;
    if (arg_buf_len == sizeof(fl_sensor_t))
    {
      len = sprintf((char*)packet_buf, "%s %ld,%d%c", fl_txt_msg_get_message_name(message_id), device_id, sensor->sensor_num, FL_TXT_MSG_TAIL);
    }
    break;
  }

  case FL_MSG_ID_BOOT_MODE:
  {
    // BMODE device_id,mode\n
    // ex) BMODE 1,1\n
    fl_boot_mode_t* bmode = (fl_boot_mode_t*)arg_buf;
    if (arg_buf_len == sizeof(fl_boot_mode_t))
    {
      len = sprintf((char*)packet_buf, "%s %ld,%d%c", fl_txt_msg_get_message_name(message_id), device_id, bmode->boot_mode, FL_TXT_MSG_TAIL);
    }
    break;
  }

  case FL_MSG_ID_RESET:
    // RESET device_id\n
    // ex) RESET 1\n
    len = sprintf((char*)packet_buf, "%s %ld%c", fl_txt_msg_get_message_name(message_id), device_id, FL_TXT_MSG_TAIL);
    break;
  }

  if (len > packet_buf_len)
  {
    len = 0;
  }

  return len;
}

FL_DECLARE(uint8_t) fl_txt_msg_build_response(
  const uint32_t device_id,
  const uint8_t message_id,
  uint8_t error,
  void* arg_buf,
  uint32_t arg_buf_len,
  uint8_t* packet_buf,
  uint32_t packet_buf_len)
{
  uint8_t len = 0;

  if ((packet_buf == NULL) ||
    (packet_buf_len == 0))
  {
    return len;
  }

  if (arg_buf != NULL)
  {
    if (arg_buf_len == 0)
    {
      return len;
    }
    else if (arg_buf_len > sizeof(fl_fw_ver_t))
    {
      return len;
    }
  }
  else
  {
    if (arg_buf != 0)
    {
      return len;
    }
  }

  if (error == FL_OK)
  {
    switch (message_id)
    {
      case FL_MSG_ID_READ_HW_VERSION:
      {
        // RHVER device_id,error,version_string\n
        // ex) RHVER 1,0,1.2.3\n
        fl_hw_ver_t* hw_ver = (fl_hw_ver_t*)arg_buf;
        if ((arg_buf_len <= sizeof(fl_hw_ver_t)) &&
            (strlen(hw_ver->version) > 0))
        {
          len = sprintf((char*)packet_buf, "%s %ld,%d,%s%c", fl_txt_msg_get_message_name(message_id), device_id, error, hw_ver->version, FL_TXT_MSG_TAIL);
        }
        break;
      }

      case FL_MSG_ID_READ_FW_VERSION:
      {
        // RFVER device_id,error,version_string\n
        // ex) RFVER 1,0,2.3.4\n
        fl_fw_ver_t* fw_ver = (fl_fw_ver_t*)arg_buf;
        if ((arg_buf_len <= sizeof(fl_fw_ver_t)) &&
          (strlen(fw_ver->version) > 0))
        {
          len = sprintf((char*)packet_buf, "%s %ld,%d,%s%c", fl_txt_msg_get_message_name(message_id), device_id, error, fw_ver->version, FL_TXT_MSG_TAIL);
        }
        break;
      }

      case FL_MSG_ID_READ_GPIO:
      {
        // RGPIO device_id,error,gpio_num,gpio_value\n
        // ex) RGPIO 1,0,2,0\n
        if (arg_buf_len == sizeof(fl_gpi_port_value_t))
        {
          fl_gpi_port_value_t* gpi_value = (fl_gpi_port_value_t*)arg_buf;
          len = sprintf((char*)packet_buf, "%s %ld,%d,%d,%d%c", fl_txt_msg_get_message_name(message_id), device_id, error, gpi_value->port_num, gpi_value->port_value, FL_TXT_MSG_TAIL);
        }
        break;
      }

      case FL_MSG_ID_WRITE_GPIO:
      case FL_MSG_ID_BOOT_MODE:
      case FL_MSG_ID_RESET:
      {
        // WGPIO device_id,error\n
        // ex) WGPIO 1,0\n
        len = sprintf((char*)packet_buf, "%s %ld,%d%c", fl_txt_msg_get_message_name(message_id), device_id, error, FL_TXT_MSG_TAIL);
        break;
      }

      case FL_MSG_ID_READ_TEMPERATURE:
      {
        // RTEMP device_id,error,sensor_num,temperature_value\n
        // ex) RTEMP 1,0,1,12.3\n
        if (arg_buf_len == sizeof(fl_temp_sensor_read_t))
        {
          fl_temp_sensor_read_t* temp_value = (fl_temp_sensor_read_t*)arg_buf;
          len = sprintf((char*)packet_buf, "%s %ld,%d,%d,%.02f%c",
            fl_txt_msg_get_message_name(message_id),
            device_id,
            error,
            temp_value->sensor_num,
            temp_value->temperature,
            FL_TXT_MSG_TAIL);
        }
        break;
      }

      case FL_MSG_ID_READ_HUMIDITY:
      {
        // RHUM device_id,error,sensor_num,temperature_value\n
        // ex) RHUM 1,0,1,45.6\n
        if (arg_buf_len == sizeof(fl_hum_sensor_read_t))
        {
          fl_hum_sensor_read_t* hum_value = (fl_hum_sensor_read_t*)arg_buf;
          len = sprintf((char*)packet_buf, "%s %ld,%d,%d,%.02f%c",
            fl_txt_msg_get_message_name(message_id),
            device_id,
            error,
            hum_value->sensor_num,
            hum_value->humidity,
            FL_TXT_MSG_TAIL);
        }
        break;
      }

      case FL_MSG_ID_READ_TEMP_AND_HUM:
      {
        // RTAH device_id,error,sensor_num,temperature_value,humidity_value\n
        // ex) RHUM 1,0,1,45.6,12.3\n
        if (arg_buf_len == sizeof(fl_temp_hum_sensor_read_t))
        {
          fl_temp_hum_sensor_read_t* temp_hum_value = (fl_temp_hum_sensor_read_t*)arg_buf;
          len = sprintf((char*)packet_buf, "%s %ld,%d,%d,%.02f,%.02f%c",
            fl_txt_msg_get_message_name(message_id),
            device_id,
            error,
            temp_hum_value->sensor_num,
            temp_hum_value->temperature,
            temp_hum_value->humidity,
            FL_TXT_MSG_TAIL);
          break;
        }
      }
    }
  }
  else
  {
    len = sprintf((char*)packet_buf, "%s %ld,%d%c", fl_txt_msg_get_message_name(message_id), device_id, error, FL_TXT_MSG_TAIL);
  }

  if (len > packet_buf_len)
  {
    len = 0;
  }

  return len;
}

FL_DECLARE(uint8_t) fl_txt_msg_build_event(
  const uint32_t device_id,
  const uint8_t message_id,
  void* arg_buf,
  uint32_t arg_buf_len,
  uint8_t* packet_buf,
  uint32_t packet_buf_len)
{
  uint8_t len = 0;

  if ((arg_buf == NULL) ||
    (packet_buf == NULL) ||
    (arg_buf_len == 0) ||
    (packet_buf_len == 0))
  {
    return len;
  }

  switch (message_id)
  {
  case FL_MSG_ID_BUTTON_EVENT:
  {
    // EBTN device_id,button_num,button_value\n
    // ex) EBTN 1,1,1\n
    if (arg_buf_len == sizeof(fl_btn_status_t))
    {
      fl_btn_status_t* btn_sts = (fl_btn_status_t*)arg_buf;
      len = sprintf((char*)packet_buf, "%s %ld,%d,%d%c",
        fl_txt_msg_get_message_name(message_id),
        device_id, btn_sts->button_num,
        btn_sts->button_value,
        FL_TXT_MSG_TAIL);
    }
    break;
  }
  }

  if (len > packet_buf_len)
  {
    len = 0;
  }

  return len;
}

FL_DECLARE(char*) fl_txt_msg_get_message_name(const uint8_t message_id)
{
  switch (message_id)
  {
  case FL_MSG_ID_READ_HW_VERSION:
    return FL_TXT_RHVER_STR;

  case FL_MSG_ID_READ_FW_VERSION:
    return FL_TXT_RFVER_STR;

  case FL_MSG_ID_READ_GPIO:
    return FL_TXT_RGPIO_STR;

  case FL_MSG_ID_WRITE_GPIO:
    return FL_TXT_WGPIO_STR;

  case FL_MSG_ID_BUTTON_EVENT:
    return FL_TXT_EBTN_STR;

  case FL_MSG_ID_READ_TEMPERATURE:
    return FL_TXT_RTEMP_STR;

  case FL_MSG_ID_READ_HUMIDITY:
    return FL_TXT_RHUM_STR;

  case FL_MSG_ID_READ_TEMP_AND_HUM:
    return FL_TXT_RTAH_STR;
    break;

  case FL_MSG_ID_BOOT_MODE:
    return FL_TXT_BMODE_STR;
    break;

  case FL_MSG_ID_RESET:
    return FL_TXT_RESET_STR;
    break;
  }

  return NULL;
}
