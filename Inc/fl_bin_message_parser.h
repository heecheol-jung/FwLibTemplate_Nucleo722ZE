// Firmware library binary message parser
#ifndef FL_BIN_MESSAGE_PARSER_H
#define FL_BIN_MESSAGE_PARSER_H

#include "fl_bin_message.h"

#define FL_BIN_MSG_PARSER_PARSING           (FL_ERROR + 1)

#define FL_BIN_MSG_PARSER_RCV_STS_STX       (0) // STX
#define FL_BIN_MSG_PARSER_RCV_STS_DEVICE_ID (1) // Device ID
#define FL_BIN_MSG_PARSER_RCV_STS_LENGTH    (2) // Remaing packet length
#define FL_BIN_MSG_PARSER_RCV_STS_HDR_DATA  (3) // Header and data

FL_BEGIN_PACK1

typedef struct _fl_bin_msg_parser
{
  // A buffer for message reception.
  uint8_t                     buf[FL_BIN_MSG_MAX_LENGTH];

  // The number of received bytes.
  uint8_t                     buf_pos;

  uint8_t                     count;

  uint8_t                     receive_state;

  void* context;

  fl_msg_cb_on_parsed_t   on_parsed_callback;

  fl_msg_dbg_cb_on_parse_started_t  on_parse_started_callback;

  fl_msg_dbg_cb_on_parse_ended_t    on_parse_ended_callback;
} fl_bin_msg_parser_t;

FL_END_PACK

FL_BEGIN_DECLS

FL_DECLARE(void) fl_bin_msg_parser_init(fl_bin_msg_parser_t* parser_handle);
FL_DECLARE(void) fl_bin_msg_parser_clear(fl_bin_msg_parser_t* parser_handle);
FL_DECLARE(fl_status_t) fl_bin_msg_parser_parse(fl_bin_msg_parser_t* parser_handle, uint8_t data, fl_bin_msg_full_t* message);

FL_END_DECLS

#endif
