require 'Qt'

class CannonField < Qt5::QWidget
    #signals 'angleChanged(int)', 'forceChanged(int)'
    #slots 'setAngle(int)', 'setForce(int)'
    
    def initialize(parent = nil)
        super
        @ang = 45
        @f = 0
        setPalette( Qt5::QPalette.new( Qt5::QColor.new( 250, 25, 20) ) )
        setAutoFillBackground(true)
    end

    def setAngle( degrees )
        if degrees < 5
            degrees = 5
        elsif degrees > 70
            degrees = 70
        end
        if @ang == degrees
            return
        end
        @ang = degrees
        repaint()
        emit angleChanged( @ang )
    end
    
    def setForce( newton )
        if newton < 0
            newton = 0
        end
        if @f == newton
            return
        end
        @f = newton
        emit forceChanged( @f )
    end

    def paintEvent( e )
        if !e.rect().intersects( cannonRect() )
            return
        end

        cr = cannonRect()
        pix = Qt5::QPixmap.new( cr.size() )
        pix.fill( self, cr.topLeft() )
        
        painter = Qt5::QPainter.new( pix )
        painter.setBrush( Qt5::QBrush.new(Qt5::blue) )
        painter.setPen( Qt5::NoPen )
        painter.translate( 0, pix.height() - 1 )
        painter.drawPie( Qt5::QRect.new(-35, -35, 70, 70), 0, 90*16 )
        painter.rotate( - @ang )
        painter.drawRect( Qt5::QRect.new(33, -4, 15, 8) )
        painter.end()
        
        painter.begin(self)
        painter.drawPixmap(cr.topLeft(), pix )
        painter.end()        
    end

    def cannonRect()
        r = Qt5::QRect.new( 0, 0, 50, 50)
        r.moveBottomLeft( rect().bottomLeft() )
        return r
    end
    
    def sizePolicy()
        return Qt5::QSizePolicy.new( Qt5::QSizePolicy::Expanding, Qt5::QSizePolicy::Expanding )
    end
end
