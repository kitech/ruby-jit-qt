
a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/../handby.so"
puts "module loaded: " + ret.to_s
