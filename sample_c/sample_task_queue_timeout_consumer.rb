#
# Task::Queue#pop(timeout_ms:) demo - consumer task.
#
# Compile with:
#   mrbc --remove-lv -Btimeout_consumer_bytecode \
#     -o sample_task_queue_timeout_consumer.c \
#     sample_task_queue_timeout_consumer.rb
#
# The consumer creates the shared queue, then pops twice with a timeout:
#   1. a short 40 ms timeout that elapses before the producer pushes, so
#      pop returns nil;
#   2. a generous 2000 ms timeout, which is satisfied when the producer
#      pushes within the window, so pop returns the value.
#
# pop(timeout_ms:) is not a busy-wait: the task is parked to WAITING with a
# deadline and the scheduler runs other tasks until either a push wakes it or
# the deadline passes.
#
$q = Task::Queue.new

puts "consumer: waiting up to 40 ms (expecting timeout)..."
v = $q.pop(timeout_ms: 40)
puts "consumer: pop returned #{v.inspect} (timed out)"

puts "consumer: waiting up to 2000 ms for a value..."
v = $q.pop(timeout_ms: 2000)
puts "consumer: got #{v}"
