#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

class SigObject < Qt5::QObject
    signals 'changed()', 'changed2(int)', 'changed3(bool)'

    def initialize()
        super()

        Qt5::rbconnectrb(self, 'changed()', self, 'onchange()');
    end
    
    def tsig1()
        emit changed();
    end

    def tsig2()
        Qt5::rbdisconnectrb(self, 'changed()', self, 'onchange()');        
    end
    
    def onchange()
        puts 'onchange slot invoked............';
    end

    def tsig3()
        Qt5::rbconnectrb(self, 'changed()', Proc.new{|| puts 'Proc slot invoked....'});
    end

    def tsig4()
        slot_onchange = lambda {|| puts 'lambda slot invoked....'};
        slot_onchange2 = ->() { puts 'lambda slot invoked2........'};
        puts slot_onchange
        puts slot_onchange2
        Qt5::rbconnectrb(self, 'changed()', slot_onchange2);
        # Qt5::rbconnectrb(self, 'changed()', ->() { puts 'lambda slot invoked3........'});
    end

    # block
    def tsig5()
        Qt5::rbconnectrb(self, 'changed()') {|| puts 'block slot invoked......'};
    end
end


def test_rbsignal()
    so = SigObject.new
    so.tsig1

    so.tsig2
    so.tsig1

    so.tsig3;
    so.tsig1

    so.tsig4
    so.tsig1

    so.tsig5
    so.tsig1
end

test_rbsignal();
