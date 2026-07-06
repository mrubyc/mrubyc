/*
 * Task::Queue#pop(timeout_ms:) demo.
 *
 * Two tasks share a single Task::Queue through the global variable $q:
 *   - the consumer task creates $q and pops twice with a timeout
 *   - the producer task sleeps, then pushes a value
 *
 * The consumer task is created first, so it runs first, creates the queue and
 * blocks on its first (40 ms) pop. The producer sleeps 120 ms -- past that
 * deadline -- so the first pop times out and returns nil. The consumer then
 * issues a second pop with a 2000 ms timeout; the producer's push arrives well
 * within that window, so the second pop returns the value.
 *
 * Expected output:
 *   consumer: waiting up to 40 ms (expecting timeout)...
 *   consumer: pop returned nil (timed out)
 *   consumer: waiting up to 2000 ms for a value...
 *   producer: pushing 42
 *   consumer: got 42
 *
 * The embedded bytecode is generated from the *.rb sources by picorbc:
 *   picorbc --remove-lv -Btimeout_consumer_bytecode \
 *     -o sample_task_queue_timeout_consumer.c \
 *     sample_task_queue_timeout_consumer.rb
 *   picorbc --remove-lv -Btimeout_producer_bytecode \
 *     -o sample_task_queue_timeout_producer.c \
 *     sample_task_queue_timeout_producer.rb
 */

#include <stdio.h>
#include <stdlib.h>
#include "mrubyc.h"

#include "sample_task_queue_timeout_consumer.c"
#include "sample_task_queue_timeout_producer.c"

#if !defined(MRBC_MEMORY_SIZE)
#define MRBC_MEMORY_SIZE (1024*60)
#endif
static uint8_t memory_pool[MRBC_MEMORY_SIZE];

int main(void)
{
  mrbc_init(memory_pool, MRBC_MEMORY_SIZE);

  // The consumer must run first so that $q exists before the producer pushes.
  if( mrbc_create_task( timeout_consumer_bytecode, 0 ) == NULL ) return 1;
  if( mrbc_create_task( timeout_producer_bytecode, 0 ) == NULL ) return 1;

  int ret = mrbc_run();

  return ret == 1 ? 0 : ret;
}
