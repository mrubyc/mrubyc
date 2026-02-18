# Note:
#  This script is supposed to run in CRuby process
#  while dispatched test is running in PicoRuby process.

require File.expand_path('../../../picoruby/mrbgems/picoruby-picotest/mrblib/picotest', __FILE__)

dir = File.expand_path(File.dirname(__FILE__))

if 0 < Picotest::Runner.new(dir).run
  exit 1
else
  exit 0
end
