require 'Qt'
#require 'pp'

class LCDRange < Qt5::QWidget
    #signals 'valueChanged(int)'
    #slots 'setValue(int)', 'setRange(int, int)', 'setText(const char *)'

    def initialize(s# , parent = nil
                  )
        super()
        #super(parent)
        init()
        setText(s)
    end
    
    def init()
        lcd = Qt5::QLCDNumber.new(2)
        @slider = Qt5::QSlider.new(Qt5::Horizontal)
        @slider.range = 0..99
        @slider.setValue(0)
        @label = Qt5::QLabel.new( ' ' )
        # @label.setAlignment( Qt5::AlignHCenter.to_i | Qt5::AlignTop.to_i )

        connect(@slider, SIGNAL('valueChanged(int)'), lcd, SLOT('display(int)'))
        connect(@slider, SIGNAL('valueChanged(int)'), SIGNAL('valueChanged(int)'))

        layout = Qt5::QVBoxLayout.new
        layout.addWidget(lcd)
        layout.addWidget(@slider)
        setLayout(layout)

        setFocusProxy(@slider)
    end

    def value()
        @slider.value()
    end

    def setValue( value )
        @slider.setValue( value )
    end
    
    def range=( r )
        setRange(r.begin, r.end)
    end

    def setRange( minVal, maxVal )
		if minVal < 0 || maxVal > 99 || minVal > maxVal
      		qWarning( "LCDRange::setRange(#{minVal},#{maxVal})\n" +
               		"\tRange must be 0..99\n" +
               		"\tand minVal must not be greater than maxVal" )
			return
		end
        @slider.range = minVal..maxVal
    end
    
    def setText( s )
        @label.setText( s )
    end

end
