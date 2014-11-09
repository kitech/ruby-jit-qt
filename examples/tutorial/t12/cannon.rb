# coding: utf-8
include Math
require 'Qt'

class CannonField < Qt5::QWidget
    
    #signals 'hit()', 'missed()', 'angleChanged(int)', 'forceChanged(int)'
    #slots 'setAngle(int)', 'setForce(int)', 'shoot()', 'moveShot()', 'newTarget()'
    
    def initialize(parent = nil)
        super
        @currentAngle = 45
        @currentForce = 0
        @timerCount = 0
        @autoShootTimer = Qt5::QTimer.new( self )
        connect( @autoShootTimer, SIGNAL('timeout()'),
                 self, SLOT('moveShot()') );
        @shootAngle = 0
        @shootForce = 0
        @target = Qt5::QPoint.new(0, 0)
        setPalette( Qt5::QPalette.new( Qt5::QColor.new( 250, 250, 200) ) )
	    setAutoFillBackground(true)
        newTarget()
        @barrelRect = Qt5::QRect.new(33, -4, 15, 8)
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
        update( cannonRect() )
        emit angleChanged( @currentAngle )
    end
    
    def setForce( newton )
        if newton < 0
            newton = 0
        end
        if @currentForce == newton
            return
        end
        @currentForce = newton
        emit forceChanged( @currentForce )
    end
    
    def shoot()
        if @autoShootTimer.isActive()
            return
        end;
        @timerCount = 0
        @shootAngle = @currentAngle
        @shootForce = @currentForce
        @autoShootTimer.start( 50 )
    end

    @@currentForceirst_time = true
    
    def newTarget()
        if @@currentForceirst_time
            @@currentForceirst_time = false
            midnight = Qt5::QTime.new( 0, 0, 0 )
            # srand( midnight.secsTo(Qt5::QTime.currentTime()) )
            # TODO 静态方法调用
        end
        r = Qt5::QRegion.new( targetRect() )
        rect = Qt5::QRect.new( 0, 0, 20, 10 )
        @target = Qt5::QPoint.new( 200 + rand(190),
                                   10  + rand(255) )
        repaint( r.united( Qt5::QRegion.new(targetRect()) ) )
    end
    
    def moveShot()
        r = Qt5::QRegion.new( shotRect() )
        @timerCount += 1

        shotR = shotRect()

        if shotR.intersects( targetRect() ) 
            @autoShootTimer.stop()
            emit hit()
        elsif shotR.x() > width() || shotR.y() > height()
            @autoShootTimer.stop()
            emit missed()
        else
            r = r.unite( Qt5::QRegion.new( shotR ) )
        end
        
        update( r )
    end

    def paintEvent( e )
        painter = Qt5::QPainter.new( self )
        paintCannon( painter )
        if @autoShootTimer.isActive()
            paintShot( painter )
        end
        paintTarget(painter)
        painter.end()
    end

    def paintShot( painter )
        painter.setPen( Qt5::NoPen )
        painter.setBrush( Qt5::QBrush.new(Qt5::black) )
        painter.drawRect( shotRect() )
    end

    def paintTarget( painter )
        painter.setBrush( Qt5::QBrush.new(Qt5::red) )
        painter.setPen( Qt5::QPen.new(Qt5::QColor.new(Qt5::black)) )
        painter.drawRect( targetRect() )
    end
    
    def paintCannon(painter)                
        painter.setPen(Qt5::NoPen)
        painter.setBrush(Qt5::QBrush.new(Qt5::blue))

        painter.save()
        painter.translate(0, height())
        painter.drawPie( Qt5::QRect.new(-35, -35, 70, 70), 0, 90*16 )
        painter.rotate( - @currentAngle )
        painter.drawRect( @barrelRect )
        painter.restore()
    end

    def cannonRect()
        r = Qt5::QRect.new( 0, 0, 50, 50)
        r.moveBottomLeft( rect().bottomLeft() )
        return r
    end
    
    def shotRect()
        gravity = 4.0

        time      = @timerCount / 4.0
        velocity  = @shootForce
        radians   = @shootAngle*3.14159265/180.0

        velx      = velocity*cos( radians )
        vely      = velocity*sin( radians )
        x0        = ( @barrelRect.right()  + 5.0 )*cos(radians)
        y0        = ( @barrelRect.right()  + 5.0 )*sin(radians)
        x         = x0 + velx*time
        y         = y0 + vely*time - 0.5*gravity*time*time

        r = Qt5::QRect.new( 0, 0, 6, 6 );
        r.moveCenter( Qt5::QPoint.new( x.round, height() - 1 - y.round ) )
        return r
    end

    def targetRect()
        r = Qt5::QRect.new( 0, 0, 20, 10 )
        r.moveCenter( Qt5::QPoint.new(@target.x(),height() - 1 - @target.y()) );
        return r
    end
end
