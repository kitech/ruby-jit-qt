#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

class SigObject < Qt5::QObject
    signals 'changed()', 'changed2(int)', 'changed3(bool)'

    def initialize()
        super()
    end
    
    def tsig1()
        emit changed();
    end
end


def test_rbsignal()
    so = SigObject.new
    so.tsig1
end

test_rbsignal();
