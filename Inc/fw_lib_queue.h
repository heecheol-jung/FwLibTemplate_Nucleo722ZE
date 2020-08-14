// fw_lib_queue.h

#ifndef FW_LIB_QUEUE_H
#define FW_LIB_QUEUE_H

#include "fw_lib_def.h"

#define FW_LIB_QUEUE_SIZE   (4)

FW_LIB_BEGIN_PACK1

typedef struct _fw_lib_queue
{
  volatile uint8_t  queue[FW_LIB_QUEUE_SIZE];

  volatile uint8_t  head;

  volatile uint8_t  tail;

  volatile uint8_t  count;
} fw_lib_queue_t;

FW_LIB_END_PACK

FW_LIB_BEGIN_DECLS

FW_LIB_DECLARE(void) fw_lib_q_init(fw_lib_queue_t* q);

FW_LIB_DECLARE(uint8_t) fw_lib_q_count(fw_lib_queue_t* q);

FW_LIB_DECLARE(fw_lib_status_t) fw_lib_q_push(fw_lib_queue_t* q, uint8_t data);

FW_LIB_DECLARE(fw_lib_status_t) fw_lib_q_pop(fw_lib_queue_t* q, uint8_t* data);

FW_LIB_END_DECLS

#endif
