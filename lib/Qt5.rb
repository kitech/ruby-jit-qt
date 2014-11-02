
a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/../handby.so"
puts "module loaded: " + ret.to_s

#RUBYLIB 
#  Additional search path for Ruby programs ($SAFE must be 0).
#DLN_LIBRARY_PATH
#  Search path for dynamically loaded modules.
#RUBYLIB_PREFIX
#  (Windows only) Mangle the RUBYLIB search path by adding this
#  prefix to each component.
# Just use $GEM_HOME (or set things up in your ~/.gemrc)
# and check that everything took with gem environment.
