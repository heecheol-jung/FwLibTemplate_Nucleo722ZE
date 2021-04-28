#include <stdlib.h>
#include "internal_util.h"
#include "fl_message_def.h"

fl_bool_t is_msg_id_char(uint8_t data)
{
  if ((data >= FL_TXT_MSG_ID_MIN_CHAR) &&
      (data <= FL_TXT_MSG_ID_MAX_CHAR))
  {
    return FL_TRUE;
  }
  else
  {
    return FL_FALSE;
  }
}

fl_bool_t is_device_id_char(uint8_t data)
{
  if ((data >= FL_TXT_DEVICE_ID_MIN_CHAR) &&
      (data <= FL_TXT_DEVICE_ID_MAX_CHAR))
  {
    return FL_TRUE;
  }
  else
  {
    return FL_FALSE;
  }
}

uint8_t get_device_id(uint8_t* buf, uint16_t buf_size)
{
  return (uint8_t)atoi((const char*)buf);
}

fl_bool_t is_tail(uint8_t data)
{
  if (data == FL_TXT_MSG_TAIL)
  {
    return FL_TRUE;
  }
  else
  {
    return FL_FALSE;
  }
}
