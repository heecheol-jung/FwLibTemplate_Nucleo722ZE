#include <string.h>
#include <stdlib.h>
#include "fw_lib_queue.h"

FW_LIB_DECLARE(void) fw_lib_q_init(fw_lib_queue_t* q)
{
  memset(q, 0, sizeof(fw_lib_queue_t));
}

FW_LIB_DECLARE(uint8_t) fw_lib_q_count(fw_lib_queue_t* q)
{
  return q->count;
}

FW_LIB_DECLARE(fw_lib_status_t) fw_lib_q_push(fw_lib_queue_t* q, uint8_t data)
{
  // Queue is full.
  if (q->count >= FW_LIB_QUEUE_SIZE)
  {
    return FW_LIB_ERROR;
  }

  if (q->head >= FW_LIB_QUEUE_SIZE)
  {
    q->head = 0;
  }

  q->queue[q->head++] = data;
  q->count++;

  return FW_LIB_OK;
}

FW_LIB_DECLARE(fw_lib_status_t) fw_lib_q_pop(fw_lib_queue_t* q, uint8_t* data)
{
  // Queue is empty.
  if (q->count == 0)
  {
    return FW_LIB_ERROR;
  }

  *data = q->queue[q->tail++];
  q->count--;

  if (q->tail >= FW_LIB_QUEUE_SIZE)
  {
    q->tail = 0;
  }

  return FW_LIB_OK;
}