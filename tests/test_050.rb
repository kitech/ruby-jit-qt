#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

### write your test code here
# test QSpacerItem
# 还是QFlags的问题

def main()
    a = Qt5::QApplication.new(ARGV.count, ARGV)

    si = Qt5::QSpacerItem.new(5, 200)
    #lo = Qt5::QLayoutItem.new()
    
    a.exec
end

main()
