#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

### write your test code here
# test QComboBox used QIcon
# 复杂对象

def main()
    a = Qt5::QApplication.new(ARGV.count, ARGV)

    cbox = Qt5::QComboBox.new
    cbox.insertItem(0, "item00000", "hintabcc1234")
    cbox.insertItem(0, "item00000", 189434356)
    cbox.insertItem(0, "item00000", true)
    cbox.insertItem(0, "item00000", rand())
    cbox.insertItem(0, "item00000", [rand(), rand(), rand()])

    #a.exec
end

main()
