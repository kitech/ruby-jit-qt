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
  a.append("aa%1bb")
  a.arg("uio") ### 参数太多，很难运行时确定。已经修正，返回值处理问题。
  a.contains("876"); 
  a.indexOf("876");  # ok
  a.toUpper; # crash ==> ok
  a.lastIndexOf("876")  # 是因为这个函数的默认参数，无法正常填入。已修正
  a.startsWith("kkkk")
  a.append("哈哈哈")
end

def test_qbytearray()
  ba = Qt5::QByteArray.new
  ba.append("yuio");
  ba.at(1) ## error, fixed nowwwwwww
  ba.count();
  ba.fill(6);  # crash, char type's process  # fixed now 
  ba.indexOf("uio");
  ba.indexOf('gggggg');
  ba.lastIndexOf("uio");
  ba.lastIndexOf('fffffff');
  ba.isEmpty();
  ba.isNull();
  ba.resize(100);
  ba.truncate(5);
  ba.toLong();  # shit _Bool  # fixed now
  ba.toLower();

end

def test_qurl()
  u = Qt5::QUrl.new
  u.setUrl("http://www.abc.com/haha/hehe.html?jie=123#mmm");
  u.port(); #???
  u.setPort(5678);
  u.toLocalFile();
  u.path(); # enum和flag默认值的问题 ==> ok
  u.query();
  u.host();
  u.setScheme("ftp");
  u.scheme();
  u.setPassword("pass123");
  u.setUserName("userabc");
  puts u.to_s;
  return;
end

def test_signal_slots()
  t = Qt5::QTimer.new
  t.interval = 50..90
  t.interval = 80
  # puts (50..90).class
  puts t.to_s();
  t.connect(t, "2timeout()", t, "1stop()");
  t.start(123); #  QObject::startTimer: Timers can only be used with threads started with QThread
end

def test_rbsignal()
    
end

def test_enum()
  puts Qt5::QFont::Bold  # 这种格式会谁接收呢？module不会接收，类也不会接收。（还是应该类会接受）
  puts Qt5::QFont::ForceIntegerMetrics
  puts Qt5::QString::SectionIncludeLeadingSep
end

def test_else()
  puts File.dirname($0)
  puts File.absolute_path(File.dirname($0))
end

def test_return()
    s = Qt5::QString.new
    puts s
    bv = s.isEmpty
    puts 'isEmpty:' + bv.to_s
    iv = s.length
    puts 'len:' + iv.to_s
    asv = s.append("123456")
    ref = asv.leftRef(3)
    slv = s.split("3")

    # puts 'len0:' + slv.count().to_s  # crash    
    #stds = s.toStdString(); # 这种调用还有太多问题

    puts 'len1:' + s.length().to_s
    ss = s.left(3);
    puts 'len2:' + ss.length().to_s
    dv = ss.toDouble()
    puts 'dv:' + dv.to_s
    lv = s.toLong
    puts 'lv:' + lv.to_s
    bav = s.toLatin1
    puts 'bav:' + bav.to_s

    #cv = s.at(2) # crash
    #puts 'cv:' + cv.to_s
    #dat = s.data() #crash
    #puts 'dat:' + dat.to_s
end

def test_lookup
    s = Qt5::QUrl.new
    # s.length()
end


# test_else;
# test_qstring
# test_qbytearray # 处理这个类问题还比较多, fixed nowwwwww
# test_qurl
# test_signal_slots
# test_enum
# test_return;
test_lookup;

