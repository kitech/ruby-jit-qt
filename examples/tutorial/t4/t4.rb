#!/usr/bin/env ruby
# coding: utf-8
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt'

app = Qt5::QApplication.new(ARGV.count, ARGV)

class MyWidget < Qt5::QWidget

  def initialize(parent = nil)
    super
    setFixedSize(200, 120)

    quit = Qt5::QPushButton.new('Quit', self)
    quit.setGeometry(62, 40, 75, 30)
    quit.setFont(Qt5::QFont.new('Times', 18, Qt5::QFont::Bold))

    connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))
end

end

widget = MyWidget.new
widget.show
app.exec
