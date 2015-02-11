#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

### write your test code here
# test return copy retrun value(structret)
# 还是返回值的问题

def main()
    a = Qt5::QApplication.new(ARGV.count, ARGV)

    label = Qt5::QLabel.new("aaaaaa")
    sp = label.sizePolicy
    p sp
    # sp is not a validate QSizePolicy object
    h = sp.hasHeightForWidth  # crash this line
    p h

    ## set and get
    sp.setHeightForWidth(true)
    h = sp.hasHeightForWidth
    p h
    
    #a.exec
end

main()
