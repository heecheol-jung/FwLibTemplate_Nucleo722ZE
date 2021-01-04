#include <string.h>
#include <stdlib.h>
#include "fw_lib_bin_message.h"
#include "fw_lib_util.h"

FW_LIB_DECLARE(uint8_t) fw_lib_bin_msg_build_command(const uint32_t device_id, const uint8_t message_id, const uint8_t sequnece_num, const uint8_t return_expected, uint8_t* packet_buf)
{
  fw_lib_bin_msg_full_t* msg_full = (fw_lib_bin_msg_full_t*)packet_buf;
  uint8_t msg_size = 0;
  uint8_t crc_offset = 0;
  uint16_t crc = 0;
  uint8_t flag = 0;

  if (message_id == FW_LIB_MSG_ID_READ_GPIO)
  {
    msg_size = sizeof(fw_bin_msg_read_gpio_cmd_t);
  }
  else if (message_id == FW_LIB_MSG_ID_WRITE_GPIO)
  {
    msg_size = sizeof(fw_bin_msg_write_gpio_cmd_t);
  }
  else if ((message_id == FW_LIB_MSG_ID_READ_TEMPERATURE) ||
           (message_id == FW_LIB_MSG_ID_READ_HUMIDITY) ||
           (message_id == FW_LIB_MSG_ID_READ_TEMP_AND_HUM))
  {
    msg_size = sizeof(fw_bin_msg_read_dht22_cmd_t);
  }
  else
  {
    msg_size = sizeof(fw_lib_bin_msg_header_t);
  }

  crc_offset = msg_size - sizeof(fw_lib_bin_msg_header_t);

  msg_full->stx = FW_LIB_BIN_MSG_STX;

  msg_full->header.device_id= FW_LIB_SWAP_4BYTES(device_id);
  msg_full->header.message_id = message_id;

  // header flag1
  flag = FW_LIB_BIT_FIELD_SET(flag, sequnece_num, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS);
  flag = FW_LIB_BIT_FIELD_SET(flag, return_expected, FW_LIB_BIN_MSG_HDR_FLG1_RET_EXPECTED_MASK, FW_LIB_BIN_MSG_HDR_FLG1_RET_EXPECTED_POS);
  flag = FW_LIB_BIT_FIELD_SET(flag, (uint8_t)FW_LIB_MSG_TYPE_COMMAND, FW_LIB_BIN_MSG_HDR_FLG1_MSG_TYPE_MASK, FW_LIB_BIN_MSG_HDR_FLG1_MSG_TYPE_POS);
  msg_full->header.flag1 = flag;

  // header flag2
  msg_full->header.flag2 = 0; // No error.

  // length field value = Message size - UID field(4) - Length field(1) + CRC(2) + ETX(1)
  msg_full->header.length = msg_size - (sizeof(uint32_t) + sizeof(uint8_t)) + 3;

  crc = fw_lib_crc_16(&packet_buf[1], msg_size);
  *((uint16_t*)&msg_full->data[crc_offset]) = FW_LIB_SWAP_2BYTES(crc);
  msg_full->data[crc_offset + 2] = FW_LIB_BIN_MSG_ETX;

  // Packet length = stx(1 byte) + message size + crc size(2 byte) + etx size(1 byte).
  return (msg_size + 4);
}

FW_LIB_DECLARE(uint8_t) fw_lib_bin_msg_build_response(const uint32_t device_id, const uint8_t message_id, const uint8_t sequnece_num, const uint8_t return_expected, uint8_t error, uint8_t* packet_buf)
{
  fw_lib_bin_msg_full_t* msg_full = (fw_lib_bin_msg_full_t*)packet_buf;
  uint8_t msg_size = 0;
  uint8_t crc_offset = 0;
  uint16_t crc = 0;
  uint8_t flag = 0;

  if (error == FW_LIB_OK)
  {
    if (message_id == FW_LIB_MSG_ID_READ_HW_VERSION)
    {
      msg_size = sizeof(fw_bin_msg_read_hw_ver_resp_t);
    }
    else if (message_id == FW_LIB_MSG_ID_READ_FW_VERSION)
    {
      msg_size = sizeof(fw_bin_msg_read_fw_ver_resp_t);
    }
    else if (message_id == FW_LIB_MSG_ID_READ_GPIO)
    {
      msg_size = sizeof(fw_bin_msg_read_gpio_resp_t);
    }
    else if ((message_id == FW_LIB_MSG_ID_READ_TEMPERATURE) ||
             (message_id == FW_LIB_MSG_ID_READ_HUMIDITY))
    {
      msg_size = sizeof(fw_bin_msg_read_dht22_resp_t);
    }
    else if (message_id == FW_LIB_MSG_ID_READ_TEMP_AND_HUM)
    {
      msg_size = sizeof(fw_bin_msg_read_dht22_temp_hum_resp_t);
    }
    else
    {
      msg_size = sizeof(fw_lib_bin_msg_header_t);
    }
  }
  else
  {
    msg_size = sizeof(fw_lib_bin_msg_header_t);
  }

  crc_offset = msg_size - sizeof(fw_lib_bin_msg_header_t);

  msg_full->stx = FW_LIB_BIN_MSG_STX;

  msg_full->header.device_id = FW_LIB_SWAP_4BYTES(device_id);
  msg_full->header.message_id = message_id;

  // header flag1
  flag = FW_LIB_BIT_FIELD_SET(flag, sequnece_num, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS);
  flag = FW_LIB_BIT_FIELD_SET(flag, return_expected, FW_LIB_BIN_MSG_HDR_FLG1_RET_EXPECTED_MASK, FW_LIB_BIN_MSG_HDR_FLG1_RET_EXPECTED_POS);
  flag = FW_LIB_BIT_FIELD_SET(flag, (uint8_t)FW_LIB_MSG_TYPE_RESPONSE, FW_LIB_BIN_MSG_HDR_FLG1_MSG_TYPE_MASK, FW_LIB_BIN_MSG_HDR_FLG1_MSG_TYPE_POS);
  msg_full->header.flag1 = flag;

  // header flag2
  flag = 0;
  flag = FW_LIB_BIT_FIELD_SET(flag, error, FW_LIB_BIN_MSG_HDR_FLG2_ERROR_MASK, FW_LIB_BIN_MSG_HDR_FLG2_ERROR_POS);
  msg_full->header.flag2 = flag;

  // msg_size - uid(4) - length(1) + crc(2) + etx(1)
  msg_full->header.length = msg_size - (sizeof(uint32_t) + sizeof(uint8_t)) + 3;

  crc = fw_lib_crc_16(&packet_buf[1], msg_size);
  *((uint16_t*)&msg_full->data[crc_offset]) = FW_LIB_SWAP_2BYTES(crc);
  msg_full->data[crc_offset + 2] = FW_LIB_BIN_MSG_ETX;

  // Packet length = stx(1 byte) + message size + crc size(2 byte) + etx size(1 byte).
  return (msg_size + 4);
}

FW_LIB_DECLARE(uint8_t) fw_lib_bin_msg_build_event(const uint32_t device_id, const uint8_t message_id, const uint8_t sequnece_num, uint8_t* packet_buf)
{
  fw_lib_bin_msg_full_t* msg_full = (fw_lib_bin_msg_full_t*)packet_buf;
  uint8_t msg_size = 0;
  uint8_t crc_offset = 0;
  uint16_t crc = 0;
  uint8_t flag = 0;

  if (message_id == FW_LIB_MSG_ID_BUTTON_EVENT)
  {
    // TODO : When error, msg_size should be sizeof(fw_lib_bin_msg_header_t).
    msg_size = sizeof(fw_bin_msg_button_evt_t);
  }
  else
  {
    return 0; // Error
  }

  crc_offset = msg_size - sizeof(fw_lib_bin_msg_header_t);

  msg_full->stx = FW_LIB_BIN_MSG_STX;

  msg_full->header.device_id = FW_LIB_SWAP_4BYTES(device_id);
  msg_full->header.message_id = message_id;

  // header flag1
  flag = FW_LIB_BIT_FIELD_SET(flag, sequnece_num, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_MASK, FW_LIB_BIN_MSG_HDR_FLG1_SEQ_NUM_POS);
  flag = FW_LIB_BIT_FIELD_SET(flag, FW_LIB_FALSE, FW_LIB_BIN_MSG_HDR_FLG1_RET_EXPECTED_MASK, FW_LIB_BIN_MSG_HDR_FLG1_RET_EXPECTED_POS);
  flag = FW_LIB_BIT_FIELD_SET(flag, (uint8_t)FW_LIB_MSG_TYPE_EVENT, FW_LIB_BIN_MSG_HDR_FLG1_MSG_TYPE_MASK, FW_LIB_BIN_MSG_HDR_FLG1_MSG_TYPE_POS);
  msg_full->header.flag1 = flag;

  // header flag2
  flag = 0;
  flag = FW_LIB_BIT_FIELD_SET(flag, FW_LIB_OK, FW_LIB_BIN_MSG_HDR_FLG2_ERROR_MASK, FW_LIB_BIN_MSG_HDR_FLG2_ERROR_POS);
  msg_full->header.flag2 = flag;

  // msg_size - uid(4) - length(1) + crc(2) + etx(1)
  msg_full->header.length = msg_size - (sizeof(uint32_t) + sizeof(uint8_t)) + 3;

  crc = fw_lib_crc_16(&packet_buf[1], msg_size);
  *((uint16_t*)&msg_full->data[crc_offset]) = FW_LIB_SWAP_2BYTES(crc);
  msg_full->data[crc_offset + 2] = FW_LIB_BIN_MSG_ETX;

  // Packet length = stx(1 byte) + message size + crc size(2 byte) + etx size(1 byte).
  return (msg_size + 4);
}
