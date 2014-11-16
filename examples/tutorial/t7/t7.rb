#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt'
#require './lcdrange.rb'
require 'lcdrange.rb'

class MyWidget < Qt5::QWidget

    def initialize(parent = nil)
        super(parent)
        quit = Qt5::QPushButton.new('Quit')
        quit.setFont(Qt5::QFont.new('Times', 18, Qt5::QFont::Bold))
    
        connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))

        grid = Qt5::QGridLayout.new
        previousRange = nil
        for row in 0..3
            for column in 0..3
                lcdRange = LCDRange.new(self)
                grid.addWidget(lcdRange, row, column)
                if previousRange != nil
                    connect( lcdRange, SIGNAL('valueChanged(int)'),
                             previousRange, SLOT('setValue(int)') )
                end
                previousRange = lcdRange
            end
        end

        layout = Qt5::QVBoxLayout.new
        layout.addWidget(quit)
        layout.addLayout(grid)
        setLayout(layout)
    end

end    

app = Qt5::QApplication.new(ARGV.count, ARGV)
$qApp = app

widget = MyWidget.new
widget.show
app.exec
