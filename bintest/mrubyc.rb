require 'tempfile'

assert('mrubyc_sample') do
  stdio = `#{cmd('mrubyc_sample')}`
  assert_equal "test\n", stdio
  assert_equal 0, $?.exitstatus
end
