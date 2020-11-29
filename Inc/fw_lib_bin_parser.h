#ifndef FW_LIB_BIN_PARSER_H
#define FW_LIB_BIN_PARSER_H

#include "fw_lib_bin_message.h"

#define FW_LIB_BIN_PARSER_PARSING           (FW_LIB_ERROR + 1)

#define FW_LIB_BIN_PARSER_RCV_STS_STX       (0) // STX
#define FW_LIB_BIN_PARSER_RCV_STS_DEVICE_ID (1) // Device ID
#define FW_LIB_BIN_PARSER_RCV_STS_LENGTH    (2) // Remaing packet length
#define FW_LIB_BIN_PARSER_RCV_STS_HDR_DATA  (3) // Header and data

typedef struct _fw_lib_bin_parser
{
  // A buffer for message reception.
  uint8_t                     buf[FW_LIB_BIN_MSG_MAX_LENGTH];

  // The number of received bytes.
  uint8_t                     buf_pos;

  uint8_t                     count;

  uint8_t                     receive_state;

  void*                       context;

  fw_lib_msg_cb_on_parsed_t   on_parsed_callback;

  fw_lib_msg_dbg_cb_on_parse_started_t  on_parse_started_callback;

  fw_lib_msg_dbg_cb_on_parse_ended_t    on_parse_ended_callback;
} fw_lib_bin_parser_t;

FW_LIB_BEGIN_DECLS

FW_LIB_DECLARE(void) fw_lib_bin_parser_init(fw_lib_bin_parser_t* parser_handle);

FW_LIB_DECLARE(void) fw_lib_bin_parser_clear(fw_lib_bin_parser_t* parser_handle);

FW_LIB_DECLARE(fw_lib_status_t) fw_lib_bin_parser_parse(fw_lib_bin_parser_t* parser_handle, uint8_t data, fw_lib_bin_msg_full_t* message);

FW_LIB_END_DECLS

#endif
