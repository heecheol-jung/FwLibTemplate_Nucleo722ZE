// Firmware library text message parser
#ifndef FL_TXT_MSG_PARSER_H
#define FL_TXT_MSG_PARSER_H

#include "fl_txt_message.h"

#define FL_TXT_MSG_PARSER_PARSING             (FL_ERROR + 1)

#define FL_TXT_MSG_PARSER_RCV_STS_MSG_ID      (0)
#define FL_TXT_MSG_PARSER_RCV_STS_DEVICE_ID   (1)
#define FL_TXT_MSG_PARSER_RCV_STS_ERROR       (2)
#define FL_TXT_MSG_PARSER_RCV_STS_DATA        (3)
#define FL_TXT_MSG_PARSER_RCV_STS_TAIL        (4)

FL_BEGIN_PACK1

typedef struct _fl_txt_msg_parser
{
  // A buffer for message reception.
  uint8_t       buf[FL_TXT_MSG_MAX_LENGTH];

  // The number of received bytes.
  uint8_t       buf_pos;

  uint8_t       receive_state;

  // Parsed message ID.
  uint8_t       msg_id;

  // Parsed device ID.
  uint32_t      device_id;

  // Response error code.
  uint8_t       error;

  // Payload buffer(fl_fw_ver_t is the longest payload).
  fl_fw_ver_t   payload;

  uint8_t       arg_count;

  void* context;

  fl_msg_cb_on_parsed_t   on_parsed_callback;
  fl_msg_dbg_cb_on_parse_started_t  on_parse_started_callback;
  fl_msg_dbg_cb_on_parse_ended_t    on_parse_ended_callback;
} fl_txt_msg_parser_t;

FL_END_PACK

FL_BEGIN_DECLS

FL_DECLARE(void) fl_txt_msg_parser_init(fl_txt_msg_parser_t* parser_handle);
FL_DECLARE(void) fl_txt_msg_parser_clear(fl_txt_msg_parser_t* parser_handle);
FL_DECLARE(fl_status_t) fl_txt_msg_parser_parse_command(fl_txt_msg_parser_t* parser_handle, uint8_t data, fl_txt_msg_t* msg_handle);
FL_DECLARE(fl_status_t) fl_txt_msg_parser_parse_response_event(fl_txt_msg_parser_t* parser_handle, uint8_t data, fl_txt_msg_t* msg_handle);
FL_DECLARE(uint8_t) fl_txt_msg_parser_get_msg_id(uint8_t* buf, uint8_t buf_size);

FL_END_DECLS

#endif