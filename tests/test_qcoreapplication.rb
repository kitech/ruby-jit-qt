#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

def slot_ruby_space()
  puts 'sllllllllllllllllllllllottttttttttt';
end


module Qt6
  # rbfunc can be Function, Proc, Lambda, Closure
  def connectrb(sender, signal, rbfunc)
    puts 'got connectrbbbbbbb'
  end

  # overload connectrb
  #def connectrb(sender, signal, rbobj, rbmth)
  #end
  
  class QObject

  end
end

# puts Qt6.class
# puts Qt6.constants
# puts Qt6.instance_methods
# exit

app = Qt5::QCoreApplication.new(ARGV.count(), ARGV)
timer = Qt5::QTimer.new
# timer.connect(timer, "2timeout()", timer, "1stopinrubyyyy()");
Qt5::connectrb(timer, "timeout()", :slot_ruby_space);
timer.start(500);

puts app
# exit;

# Signal.trap("INT") do
#   $debug = !$debug
#   puts "Debug now: #$debug"
# end

# Thread.new {
#   puts self
#   # app.exec # QCoreApplication::exec: Must be called from the main thread
#   # puts "execed"
# #   sleep(5);
# #   app.quit
#   100.times {
#     sleep(1)
#     puts "kjoiefwaf"
#     app.exec
#   }
# };

# sleep(5)
# app.quit
app.exec  # 为什么不能接收控制台的Ctrl+c信号呢?

# 5.times {
#   sleep(1);
# }
# app.quit






