#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

require 'benchmark'

a = File.absolute_path(File.dirname(__FILE__))
puts Benchmark.measure {
    ret = require "#{a}/bc_init.rb"
}
### write your test code here

puts Benchmark.measure {
    # 100    8.380000   8.670000  17.050000 ( 26.167677)
    # 10     0.900000   0.840000   1.740000 (  2.620329)
    # 1      0.180000   0.080000   0.260000 (  0.326139)
    1.times {
        s = Qt5::QString.new("aaa")
    }
}

puts Benchmark.measure {
    # 100    8.380000   8.670000  17.050000 ( 26.167677)
    # 10     0.900000   0.840000   1.740000 (  2.620329)
    # 1      0.180000   0.080000   0.260000 (  0.326139)
    1.times {
        s = Qt5::QString.new("aaa")
    }
}


