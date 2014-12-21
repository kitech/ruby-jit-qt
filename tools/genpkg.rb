#!/usr/bin/env ruby
# coding: utf-8

# 从当前目录生成二进制包，安装到$HOME/.gem/qt5ruby
# qt5ruby/lib
# qt5ruby/bin
# qt5ruby/libexec
# 开始周期内，可以考虑使用软链接。

def esystem(cmd)
    cmd = "set -x;" + cmd;
    return system(cmd);
end

home = ENV['HOME'];
esystem("mkdir -pv #{home}/.gem/qt5ruby/lib");
esystem("mkdir -pv #{home}/.gem/qt5ruby/libexec");
esystem("mkdir -pv #{home}/.gem/qt5ruby/bin");

pwd = ENV['PWD'];
esystem("ln -sv #{pwd}/lib/* #{home}/.gem/qt5ruby/lib");
esystem("ln -sv #{pwd}/libhandy.so #{home}/.gem/qt5ruby/libexec");

puts 'Done';


