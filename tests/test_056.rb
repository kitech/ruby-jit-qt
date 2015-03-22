#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

### write your test code here
# only test basic gui app

def main()
    a = Qt5::QApplication.new(ARGV.count, ARGV)
    w = Qt5::QWidget.new()
    w.show()
    a.exec
end

main()
