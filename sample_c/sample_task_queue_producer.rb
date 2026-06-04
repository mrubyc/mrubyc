#
# Task::Queue blocking pop demo - producer task.
#
# Compile with:
#   picorbc --remove-lv -Bproducer_bytecode -o sample_task_queue_producer.c \
#     sample_task_queue_producer.rb
#
# The producer pushes one value into the shared queue created by the consumer.
# The push wakes the consumer that is blocked on pop.
#
puts "producer: pushing 42"
$q.push(42)
