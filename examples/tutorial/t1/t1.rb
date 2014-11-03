#!/usr/bin/env ruby
# coding: utf-8
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt'

a = Qt5::QApplication.new(ARGV.count, ARGV)
hello = Qt5::QPushButton.new('Hello World!')
#hello = Qt5::QPushButton.new('Hello World!', nil)  # crash, 这个归为bug，先不考虑
hello.resize(100, 30)
hello.show()
a.exec()
