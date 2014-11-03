#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt';

a = Qt5::QApplication.new(ARGV.count, ARGV)

# quit = Qt::PushButton.new('Quit', nil)
quit = Qt5::QPushButton.new('Quit')
quit.resize(75, 30)
font = Qt5::QFont.new('Times', 18, Qt5::QFont::Bold)
# quit.setFont(Qt5::QFont.new('Times', 18, Qt5::QFont::Bold))  # setFont method not found

#Qt::Object.connect(quit, SIGNAL('clicked()'), a, SLOT('quit()'))
quit.connect(quit, SIGNAL('clicked()'), a, SLOT('quit()'))

quit.show
a.exec
exit
