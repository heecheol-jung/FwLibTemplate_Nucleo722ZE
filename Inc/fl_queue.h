// Firmware library queue
// fl_queue.h

#ifndef FL_QUEUE_H
#define FL_QUEUE_H

#include "fl_def.h"

#define FL_QUEUE_SIZE   (32)

FL_BEGIN_PACK1

typedef struct _fl_queue
{
  volatile uint8_t  queue[FL_QUEUE_SIZE];

  volatile uint8_t  head;

  volatile uint8_t  tail;

  volatile uint8_t  count;
} fl_queue_t;

FL_END_PACK

FL_BEGIN_DECLS

FL_DECLARE(void) fl_q_init(fl_queue_t* q);
FL_DECLARE(uint8_t) fl_q_count(fl_queue_t* q);
FL_DECLARE(fl_status_t) fl_q_push(fl_queue_t* q, uint8_t data);
FL_DECLARE(fl_status_t) fl_q_pop(fl_queue_t* q, uint8_t* data);

FL_END_DECLS

#endif
