/*
 * Task::Queue blocking pop demo.
 *
 * Two tasks share a single Task::Queue through the global variable $q:
 *   - the consumer task creates $q and blocks on $q.pop
 *   - the producer task pushes a value into $q, which wakes the consumer
 *
 * The consumer task is created first, so it runs first, creates the queue and
 * blocks; the scheduler then runs the producer, whose push wakes the consumer.
 *
 * Expected output:
 *   consumer: waiting for an item...
 *   producer: pushing 42
 *   consumer: got 42
 *
 * The embedded bytecode is generated from the *.rb sources by picorbc:
 *   picorbc --remove-lv -Bconsumer_bytecode -o sample_task_queue_consumer.c \
 *     sample_task_queue_consumer.rb
 *   picorbc --remove-lv -Bproducer_bytecode -o sample_task_queue_producer.c \
 *     sample_task_queue_producer.rb
 */

#include <stdio.h>
#include <stdlib.h>
#include "mrubyc.h"

#include "sample_task_queue_consumer.c"
#include "sample_task_queue_producer.c"

#if !defined(MRBC_MEMORY_SIZE)
#define MRBC_MEMORY_SIZE (1024*60)
#endif
static uint8_t memory_pool[MRBC_MEMORY_SIZE];

int main(void)
{
  mrbc_init(memory_pool, MRBC_MEMORY_SIZE);

  // The consumer must run first so that $q exists before the producer pushes.
  if( mrbc_create_task( consumer_bytecode, 0 ) == NULL ) return 1;
  if( mrbc_create_task( producer_bytecode, 0 ) == NULL ) return 1;

  int ret = mrbc_run();

  return ret == 1 ? 0 : ret;
}
