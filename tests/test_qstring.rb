#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

def test_qstring()
  
  a = Qt5::QString.new

  puts "==========";

  puts a.length()
  a.append("876")

  puts a.length()

  puts a.isEmpty();

  puts a.length();

  a.append("%1")

  a.arg("uio") ### 参数太多，很难运行时确定。已经修正，返回值处理问题。

  a.contains("876");

  a.indexOf("876");  # ok

  a.toUpper; # crash
  a.lastIndexOf("876")  # 是因为这个函数的默认参数，无法正常填入。已修正

  a.startsWith("kkkk")


  a.append("哈哈哈")

end

def test_qbytearray()
  ba = Qt5::QByteArray.new
  ba.append("yuio");
  ba.count();
  # ba.fill(6);  # crash, char type's process
  ba.indexOf("uio");
  ba.indexOf('gggggg');
  ba.lastIndexOf("uio");
  ba.lastIndexOf('fffffff');
  ba.isEmpty();
  ba.isNull();
  ba.resize(100);
  ba.truncate(5);
  # ba.toLong();  # shit _Bool
  ba.at(1)
  ba.toLower();
end

def test_qurl()
  u = Qt5::QUrl.new
  u.setUrl("http://www.abc.com/hehe?jie=123#mmm");
  #u.port(); #???
  u.setPort(5678);
  u.toLocalFile();
  #u.path(); # enum和flag默认值的问题
  u.scheme();
  puts u.to_s;
  return;
end


# test_qstring
# test_qbytearray
test_qurl



