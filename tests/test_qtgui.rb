#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"
ret = require "#{a}/Qt5.rb"

class MyRubyThreadFix
  def slot_ruby_space()
    puts 'classssssssssllllllllllllllllllllllottttttttttt';
  end
end


app = Qt5::QApplication.new(ARGV.count, ARGV);
#timer = Qt5::QTimer.new
# timer.connect(timer, "2timeout()", timer, "1stopinrubyyyy()");
# Qt5::connectrb(timer, "timeout()", :slot_ruby_space);
#thfix = MyRubyThreadFix.new
# Qt5::connectrb(timer, "timeout()", thfix, :slot_ruby_space);
# exit;
# timer.start(500);
# Qt5::RubyThreadFix.new

puts app
# exit;

def test_widget()
  w = Qt5::QWidget.new
  w.show()
end

# okkk
def test_range()
    slider = Qt5::QSlider.new(Qt5::Horizontal)
    slider.range = 1..99
end

def test_disp()
    hits = Qt5::QLCDNumber.new( 2, self )
    puts hits.value()
    hits.display('123');  # display 和 ruby object的方法冲突了？？？确定
    puts "============456"
    exit;
    
    #display('aaaaaa')
end

#test_widget;
#test_range;
test_disp;

#app.exec()




