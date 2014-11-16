#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt'

class LCDRange < Qt5::QWidget

def initialize(parent = nil)
    super
    lcd = Qt5::QLCDNumber.new(2)
    slider = Qt5::QSlider.new(Qt5::Horizontal)
    slider.range = 0..99
    slider.value = 0

    lcd.connect(slider, SIGNAL('valueChanged(int)'), SLOT('display(int)'))

    layout = Qt5::QVBoxLayout.new
    layout.addWidget(lcd)
    layout.addWidget(slider)
    setLayout(layout)
end

end

class MyWidget < Qt5::QWidget

def initialize()
    super
    quit = Qt5::QPushButton.new('Quit')
    quit.setFont(Qt5::QFont.new('Times', 18, Qt5::QFont::Bold))
    connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))

    grid = Qt5::QGridLayout.new
	
    for row in 0..3
        for column in 0..3
            grid.addWidget(LCDRange.new, row, column)
        end
    end

    layout = Qt5::QVBoxLayout.new
    layout.addWidget(quit)
    layout.addLayout(grid)
    setLayout(layout)
end

end    

app = Qt5::QApplication.new(ARGV.count, ARGV)
widget = MyWidget.new
widget.show
app.exec
