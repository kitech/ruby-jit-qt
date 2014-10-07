#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

a = Qt5::QString.new

puts "==========";

# puts a.length()
a.append("876")

# puts a.length()

# puts a.isEmpty();

# puts a.length();

# a.append("%1")

# a.arg("uio") ### 参数太多，很难运行时确定。

# a.contains("876");

# a.indexOf("876");  # ok

# a.toUpper; # crash
# a.lastIndexOf("876")  # 是因为这个函数的默认参数，无法正常填入。已修正

# a.startsWith("kkkk")


# s = Qt5::QString.new

# puts s

# puts s.length

# s.append("abc")

# puts s.length

# puts s.append("456789")

# puts s.length

# puts s.toUpper()

# s.append("%1")

# puts s.arg(9999)

# puts s.contains("123")

# puts s.contains("456")

# s.append("哈哈哈")

# puts s.to_s


