#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"
ret = require "#{a}/Qt5.rb"

class MyRubyThreadFix
  def slot_ruby_space()
    puts 'classssssssssllllllllllllllllllllllottttttttttt';
  end
end


app = Qt5::QApplication.new(ARGV.count, ARGV);
#timer = Qt5::QTimer.new
# timer.connect(timer, "2timeout()", timer, "1stopinrubyyyy()");
# Qt5::connectrb(timer, "timeout()", :slot_ruby_space);
#thfix = MyRubyThreadFix.new
# Qt5::connectrb(timer, "timeout()", thfix, :slot_ruby_space);
# exit;
# timer.start(500);
# Qt5::RubyThreadFix.new

puts app
# exit;

def test_widget()
  w = Qt5::QWidget.new
  w.show()
end

### TODO QGridLayout not normal.
def test_layout()
    w = Qt5::QWidget.new
    #w.show()
    
    b = Qt5::QPushButton.new("Quit...", w);
    b2 = Qt5::QPushButton.new("Quit2222...", w);
    vlo = Qt5::QVBoxLayout.new();
    w2 = Qt5::QLCDNumber.new(w);
    w3 = Qt5::QLCDNumber.new(w);

    w2.display('6000');
    w3.display('200');
    
    vlo.addWidget(w2);
    vlo.addWidget(w3);

    #w.setLayout(vlo)

    glo = Qt5::QGridLayout.new();
    glo.addWidget(b, 0, 0)    
    glo.addLayout(vlo, 1, 0); ## 没有执行？？？
    glo.addWidget(b2, 1, 1)


    #glo.columnCount();    
    # glo.rowCount();    # 这里如果>1，则成功了。
    #exit;
    
    #w.layout(); # ok
    w.setLayout(glo);
    #w.layout(); # ok
    #w.hide();


    w.show();
    #glo.activate();
    #w.resize(200, 100)
    #glo.count()    
    #glo.dumpObjectInfo();
    #w.dumpObjectTree();
end

# okkk
def test_range()
    slider = Qt5::QSlider.new(Qt5::Horizontal)
    slider.range = 1..99
end

def test_disp()
    hits = Qt5::QLCDNumber.new( 2, self )
    puts hits.value()
    hits.display('123');  # display 和 ruby object的方法冲突了？？？确定
    puts "============456"
    exit;
    
    #display('aaaaaa')
end

def test_qApp()
    puts '========'
    puts $qApp;
    puts '========'    
end


test_layout;
#test_qApp;
#test_widget;
#test_range;
# test_disp;

app.exec()




