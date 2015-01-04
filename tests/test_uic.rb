#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

require "#{a}/ui_MainWindow.rb"

class MyMainWindow < Qt5::QMainWindow
    def initialize(parent = nil)
        super(parent)

        @ui = Ui::MainWindow.new
        @ui.setupUi(self);
    end
end

def test_uic()
    a = Qt5::QApplication.new(ARGV.count(), ARGV)
    w = MyMainWindow.new
    w.show();
    #r = Qt5::QApplication.translate("MainWindow", "MainWindow", nil)  # OKed
    #r = Qt5::QApplication.translate("MainWindow", "efggggggg", nil)
    #puts r;
    a.exec();
end

test_uic();

