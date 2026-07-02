#
# Task::Queue#pop(timeout_ms:) demo - producer task.
#
# Compile with:
#   mrbc --remove-lv -Btimeout_producer_bytecode \
#     -o sample_task_queue_timeout_producer.c \
#     sample_task_queue_timeout_producer.rb
#
# The producer sleeps past the consumer's first (40 ms) timeout so that pop
# returns nil, then pushes one value. The push arrives well within the
# consumer's second (2000 ms) deadline, so that pop returns the value.
#
sleep_ms 120
puts "producer: pushing 42"
$q.push(42)
