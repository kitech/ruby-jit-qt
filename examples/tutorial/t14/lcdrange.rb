class LCDRange < Qt5::QWidget

    #signals 'valueChanged(int)'

    #slots 'value=(int)', 'setRange(int, int)', 'text=(const char*)'
    
    def initialize(s, parent = nil)
        #super(parent)
        super()
        init()
        #self.text = s
    end
    
    def init
        lcd = Qt5::QLCDNumber.new(2)
        @slider = Qt5::QSlider.new(Qt5::Horizontal) do |s|
            s.range = 0..99
            s.value = 0
        end
        
        @label = Qt5::QLabel.new do |l|
            #l.alignment = Qt5::QAlignHCenter.to_i | Qt5::QAlignTop.to_i
            #l.setSizePolicy(Qt5::QSizePolicy::Preferred, Qt5::QSizePolicy::Fixed)
        end
            
        connect(@slider, SIGNAL('valueChanged(int)'), lcd, SLOT('display(int)'))
        connect(@slider, SIGNAL('valueChanged(int)'), SIGNAL('valueChanged(int)'))
        
        self.layout = Qt5::QVBoxLayout.new do |l|
            l.addWidget(lcd)
            l.addWidget(@slider)
            l.addWidget(@label)
        end
        
        setFocusProxy(@slider)
    end
    
    def value
        @slider.value
    end

    def value=( value )
        @slider.value = value
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
        @slider.setRange( minVal, maxVal )
    end
    
    def text=( s )
        @label.text = s
    end

end
