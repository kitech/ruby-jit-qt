#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt'

a = Qt5::QApplication.new(ARGV.count, ARGV)
hello = Qt5::QPushButton.new('Hello World!')
#hello = Qt5::QPushButton.new('Hello World!', nil)  # crash
hello.resize(100, 30)   #crash
hello.show()
a.exec()
