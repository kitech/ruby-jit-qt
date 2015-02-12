#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

### write your test code here
# test name resolution: void* => class object
# 函数名字决议的问题

def main()
    a = Qt5::QApplication.new(ARGV.count, ARGV)

    label = Qt5::QLabel.new("aaaaaa")
    sp = label.sizePolicy
    p sp
    label.sizePolicy = sp
    #a.exec
end

main()
