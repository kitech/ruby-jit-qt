require 'Qt'

class CannonField < Qt5::QWidget
    #signals 'angleChanged(int)'
    #slots 'setAngle(int)'
    
    def initialize(parent = nil)
        super
        @currentAngle = 45
        setPalette( Qt5::QPalette.new( Qt5::QColor.new( 250, 250, 200) ) )
	    setAutoFillBackground(true)
    end

    def setAngle( degrees )
        if degrees < 5
            degrees = 5
        elsif degrees > 70
            degrees = 70
        end
        if @currentAngle == degrees
            return
        end
        @currentAngle = degrees
        repaint()
        emit angleChanged( @currentAngle )
    end

    def paintEvent( event )
        painter = Qt5::QPainter.new( self )

        painter.setPen( Qt5::NoPen )
        painter.setBrush( Qt5::QBrush.new(Qt5::blue) )

        painter.translate( 0, rect().bottom() )
        painter.drawPie( Qt5::QRect.new(-35, -35, 70, 70), 0, 90*16 )
        painter.rotate( - @currentAngle )
        painter.drawRect( Qt5::QRect.new(33, -4, 15, 8) )
        painter.end()
    end


    def sizePolicy()
        return Qt5::QSizePolicy.new( Qt5::QSizePolicy::Expanding, Qt5::QSizePolicy::Expanding )
    end
end
