# Note:
#  This script is supposed to run in CRuby process
#  while dispached test is running in PicoRuby process.

require File.expand_path('../../picoruby/mrbgems/picoruby-picotest/mrblib/picotest', __FILE__)

dir = File.expand_path(File.dirname(__FILE__))

Picotest::Runner.new(dir).run
