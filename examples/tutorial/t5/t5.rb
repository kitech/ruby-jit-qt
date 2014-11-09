#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt'

class MyWidget < Qt5::QWidget

def initialize()
    super
    quit = Qt5::QPushButton.new('Quit')
    #quit.setFont(Qt5::QFont.new('Times', 18, Qt5::QFont::Bold))
    
    connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))
    
    lcd = Qt5::QLCDNumber.new(2)

    slider = Qt5::QSlider.new(Qt5::Horizontal)
    # slider.range = 0..99
    # slider.value = 0
    slider.setRange(0, 99)
    slider.setValue(15)

    connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))
    connect(slider, SIGNAL('valueChanged(int)'),
            lcd, SLOT('display(int)'))

    layout = Qt5::QVBoxLayout.new
    layout.addWidget(quit)
    layout.addWidget(lcd)
    layout.addWidget(slider)
    setLayout(layout)
end

end

app = Qt5::QApplication.new(ARGV.count, ARGV)
$qApp = app

widget = MyWidget.new
widget.show
app.exec
