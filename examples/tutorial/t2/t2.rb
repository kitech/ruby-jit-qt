#!/usr/bin/env ruby
# coding: utf-8
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt';

a = Qt5::QApplication.new(ARGV.count, ARGV)

quit = Qt5::QPushButton.new('Quit', nil)
quit.resize(75, 30)
quit.setFont(Qt5::QFont.new('Times', 18, Qt5::QFont::Bold))

puts "=================1111111111111"
#Qt::Object.connect(quit, SIGNAL('clicked()'), a, SLOT('quit()'))
#这种调用要怎么截获，使用rb_define_singleton_method
# 两种方式都可以使用。
Qt5::QObject.connect(quit, SIGNAL('clicked()'), a, SLOT('quit()'))
#quit.connect(quit, SIGNAL('clicked()'), a, SLOT('quit()'))

quit.show
a.exec
exit
