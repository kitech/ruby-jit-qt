#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt'

app = Qt5::QApplication.new(ARGV.count, ARGV)

window = Qt5::QWidget.new

# exit;

window.resize(200, 120)


quit = Qt5::QPushButton.new('Quit', window)
#quit.font = Qt5::QFont.new('Times', 18, Qt5::QFont::Bold)
#quit.setGeometry(10, 40, 180, 40)
app.connect(quit, SIGNAL('clicked()'), app, SLOT('quit()'))

window.show
app.exec

