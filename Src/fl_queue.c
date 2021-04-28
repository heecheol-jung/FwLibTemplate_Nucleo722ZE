#include <string.h>
#include <stdlib.h>
#include "fl_queue.h"

FL_DECLARE(void) fl_q_init(fl_queue_t* q)
{
  memset(q, 0, sizeof(fl_queue_t));
}

FL_DECLARE(uint8_t) fl_q_count(fl_queue_t* q)
{
  return q->count;
}

FL_DECLARE(fl_status_t) fl_q_push(fl_queue_t* q, uint8_t data)
{
  // Queue is full.
  if (q->count >= FL_QUEUE_SIZE)
  {
    return FL_ERROR;
  }

  if (q->head >= FL_QUEUE_SIZE)
  {
    q->head = 0;
  }

  q->queue[q->head++] = data;
  q->count++;

  return FL_OK;
}

FL_DECLARE(fl_status_t) fl_q_pop(fl_queue_t* q, uint8_t* data)
{
  // Queue is empty.
  if (q->count == 0)
  {
    return FL_ERROR;
  }

  *data = q->queue[q->tail++];
  q->count--;

  if (q->tail >= FL_QUEUE_SIZE)
  {
    q->tail = 0;
  }

  return FL_OK;
}
