#!/usr/bin/ruby -w
require 'Qt'

class LCDRange < Qt5::QWidget
    #signals 'valueChanged(int)'
    #slots 'setValue(int)'

    def initialize(parent = nil)
        super
        lcd = Qt5::QLCDNumber.new(2)

        @slider = Qt5::QSlider.new(Qt5::QHorizontal)
        @slider.range = 0..99
        @slider.value = 0

        connect(@slider, SIGNAL('valueChanged(int)'), lcd, SLOT('display(int)'))
        connect(@slider, SIGNAL('valueChanged(int)'), SIGNAL('valueChanged(int)'))
   
        layout = Qt5::QVBoxLayout.new
        layout.addWidget(lcd)
        layout.addWidget(@slider)
        setLayout(layout)
    end

    def value()
        @slider.value()
    end

    def setValue( value )
        @slider.setValue( value )
    end
end
