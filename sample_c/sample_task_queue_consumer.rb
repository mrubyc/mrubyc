#
# Task::Queue blocking pop demo - consumer task.
#
# Compile with:
#   picorbc --remove-lv -Bconsumer_bytecode -o sample_task_queue_consumer.c \
#     sample_task_queue_consumer.rb
#
# The consumer creates the shared queue first, then blocks on pop until the
# producer task pushes a value. pop() is not a busy-wait: the task is moved to
# the WAITING state and the scheduler runs other tasks until a push wakes it.
#
$q = Task::Queue.new
puts "consumer: waiting for an item..."
v = $q.pop
puts "consumer: got #{v}"
