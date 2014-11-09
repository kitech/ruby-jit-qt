### RUBYLIB env variable
$VERBOSE = false; $:.unshift File.absolute_path(File.dirname($0))

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

### from qtbindings/lib/Qt/qtruby4.rb
class Object
  def SIGNAL(signal)
    if signal.kind_of? Symbol
      return "2" + signal.to_s + "()"
    else
      return "2" + signal
    end
  end

  def SLOT(slot)
    if slot.kind_of? Symbol
      return "1" + slot.to_s + "()"
    else
      return "1" + slot
    end
  end

  def emit(signal)
    return signal
  end

  def QT_TR_NOOP(x) x end
  def QT_TRANSLATE_NOOP(scope, x) x end

  # See the discussion here: http://eigenclass.org/hiki.rb?instance_exec
  # about implementations of the ruby 1.9 method instance_exec(). This
  # version is the one from Rails. It isn't thread safe, but that doesn't
  # matter for the intended use in invoking blocks as Qt slots.
  def instance_exec(*arguments, &block)
    block.bind(self)[*arguments]
  end unless defined? instance_exec
end
