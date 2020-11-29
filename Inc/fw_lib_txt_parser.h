#ifndef FW_LIB_TXT_PARSER_H
#define FW_LIB_TXT_PARSER_H

#include "fw_lib_txt_message.h"

#define FW_LIB_TXT_MSG_ID_MIN_CHAR            ('A')

#define FW_LIB_TXT_MSG_ID_MAX_CHAR            ('Z')

#define FW_LIB_TXT_DEVICE_ID_MIN_CHAR         ('0')

#define FW_LIB_TXT_DEVICE_ID_MAX_CHAR         ('9')

#define FW_LIB_TXT_MSG_ID_MAX_LEN             (5)

#define FW_LIB_TXT_DEVICE_ID_MAX_LEN          (2)

// Delimiter for a message id and device id
#define FW_LIB_TXT_MSG_ID_DEVICE_ID_DELIMITER (' ')

// Delimiter for arguments.
#define FW_LIB_TXT_MSG_ARG_DELIMITER          (',')

#define FW_LIB_TXT_PARSER_PARSING             (FW_LIB_ERROR + 1)

#define FW_LIB_TXT_PARSER_RCV_STS_MSG_ID      (0)
#define FW_LIB_TXT_PARSER_RCV_STS_DEVICE_ID   (1)
#define FW_LIB_TXT_PARSER_RCV_STS_DATA        (2)
#define FW_LIB_TXT_PARSER_RCV_STS_TAIL        (3)

FW_LIB_BEGIN_PACK1

typedef struct _fw_lib_txt_parser
{
  // A buffer for message reception.
  uint8_t                     buf[FW_LIB_TXT_MSG_MAX_LENGTH];

  // The number of received bytes.
  uint8_t                     buf_pos;

  uint8_t                     receive_state;

  // Parsed message ID.
  uint8_t                     msg_id;

  // Parsed device ID.
  uint32_t                    device_id;

  // Parsed arguments.
  fw_lib_msg_arg_t            args[FW_LIB_TXT_MSG_MAX_ARG_COUNT];

  // The number of parsed arguments.
  uint8_t                     arg_count;

  void*                       context;

  fw_lib_msg_cb_on_parsed_t   on_parsed_callback;

  fw_lib_msg_dbg_cb_on_parse_started_t  on_parse_started_callback;

  fw_lib_msg_dbg_cb_on_parse_ended_t    on_parse_ended_callback;
} fw_lib_txt_parser_t;

FW_LIB_END_PACK

FW_LIB_BEGIN_DECLS

FW_LIB_DECLARE(void) fw_lib_txt_parser_init(fw_lib_txt_parser_t* parser_handle);

FW_LIB_DECLARE(void) fw_lib_txt_parser_clear(fw_lib_txt_parser_t* parser_handle);

FW_LIB_DECLARE(fw_lib_status_t) fw_lib_txt_parser_parse_command(fw_lib_txt_parser_t* parser_handle, uint8_t data, fw_lib_txt_msg_t* msg_handle);

FW_LIB_DECLARE(fw_lib_status_t) fw_lib_txt_parser_parse_response_event(fw_lib_txt_parser_t* parser_handle, uint8_t data, fw_lib_txt_msg_t* msg_handle);

FW_LIB_DECLARE(uint8_t) fw_lib_txt_parser_get_msg_id(uint8_t* buf, uint8_t buf_size);

FW_LIB_END_DECLS

#endif

