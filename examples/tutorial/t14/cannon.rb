include Math

class CannonField < Qt5::QWidget

    #signals 'hit()', 'missed()', 'angleChanged(int)', 'forceChanged(int)',
    #        'canShoot(bool)'

    #slots   'angle=(int)', 'force=(int)', 'shoot()', 'moveShot()',
    #        'newTarget()', 'setGameOver()', 'restartGame()'

    def initialize(# parent = nil
                  )
        super()
        @currentAngle = 45
        @currentForce = 0
        @timerCount = 0;
        @autoShootTimer = Qt5::QTimer.new( self )
        connect( @autoShootTimer, SIGNAL('timeout()'),
                 self, SLOT('moveShot()') )
        @shootAngle = 0
        @shootForce = 0
        @target = Qt5::QPoint.new(0, 0)
        @gameEnded = false
        @barrelPressed = false
        setPalette( Qt5::QPalette.new( Qt5::QColor.new( 250, 250, 200) ) )
        setAutoFillBackground(true)
        newTarget()
        @barrelRect = Qt5::QRect.new(30, -5, 20, 10)
    end

    def angle() 
        return @currentAngle 
    end

    def force() 
        return @currentForce 
    end

    def gameOver() 
        return @gameEnded 
    end

    def angle=( degrees )
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
        #emit angleChanged( @currentAngle )
    end
    
    def force=( newton )
        if newton < 0
            newton = 0
        end
        if @currentForce == newton
            return
        end
        @currentForce = newton
        #emit forceChanged( @currentForce )
    end
    
    def shoot()
        if shooting?
            return
        end
        @timerCount = 0
        @shootAngle = @currentAngle
        @shootForce = @currentForce
        @autoShootTimer.start( 25 )
        #emit canShoot( false )
    end

    @@first_time = true
    
    def newTarget()
        if @@first_time
            @@first_time = false
            midnight = Qt5::QTime.new( 0, 0, 0 )
            #srand( midnight.secsTo(Qt5::QTime.currentTime()) )
        end
        @target = Qt5::QPoint.new( 200 + rand(190), 10  + rand(255) )
        update()
    end
    
    def setGameOver()
        if @gameEnded
            return
        end
        if shooting?
            @autoShootTimer.stop()
        end
        @gameEnded = true
        update()
    end

    def restartGame()
        if shooting?
            @autoShootTimer.stop()
        end
        @gameEnded = false
        update()
        #emit canShoot( true )
    end
    
    def moveShot()
        r = Qt5::QRegion.new( shotRect() )
        @timerCount += 1

        shotR = shotRect()

        if shotR.intersects( targetRect() ) 
            @autoShootTimer.stop()
            emit hit()
            #emit canShoot(true)
        elsif shotR.x() > width() || shotR.y() > height() ||
                    shotR.intersects(barrierRect()) 
            @autoShootTimer.stop()
            emit missed()
            #emit canShoot(true)
        else
            r = r.unite( Qt5::QRegion.new( shotR ) )
        end
        
        update( r )
    end
    private :moveShot
	
    def mousePressEvent( e )
        if e.button() != Qt5::QLeftButton
            return
        end
        if barrelHit( e.pos() )
            @barrelPressed = true
        end
    end

    def mouseMoveEvent( e )
        if !@barrelPressed
            return
        end
        pnt = e.pos();
        if pnt.x() <= 0
            pnt.setX( 1 )
        end
        if pnt.y() >= height()
            pnt.setY( height() - 1 )
        end
        rad = atan2((rect().bottom()-pnt.y()), pnt.x())
        self.angle = ( rad*180/3.14159265 ).round
    end

    def mouseReleaseEvent( e )
        if e.button() == Qt5::QLeftButton
            @barrelPressed = false
        end
    end

    def paintEvent( e )
        painter = Qt5::QPainter.new( self )

        if @gameEnded
            painter.pen = Qt5::QColor.new(Qt5::Qblack)
            painter.font = Qt5::QFont.new( 'Courier', 48, Qt5::QFont::Bold )
            painter.drawText( rect(), Qt5::QAlignCenter, 'Game Over' )
        end
        paintCannon( painter )
        paintBarrier( painter )
        if shooting?
            paintShot( painter )
        end       
        if !@gameEnded
            paintTarget( painter )
        end
        
        painter.end
    end

    def paintShot( painter )
        painter.pen = Qt5::NoPen
        painter.brush = Qt5::Brush.new(Qt5::black)
        painter.drawRect( shotRect() )
    end
    
    def paintTarget( painter )
        painter.brush = Qt5::QBrush.new(Qt5::red)
        painter.pen = Qt5::QColor.new(Qt5::black)
        painter.drawRect( targetRect() )
    end
    
    def paintBarrier( painter )
        painter.brush = Qt5::QBrush.new(Qt5::yellow)
        painter.pen = Qt5::QColor.new(Qt5::black)
        painter.drawRect( barrierRect() )
    end
    
    def paintCannon( painter )
        painter.pen = Qt5::NoPen
        painter.brush = Qt5::QBrush.new(Qt5::blue)

        painter.save
        painter.translate(0, height())
        painter.drawPie( Qt5::QRect.new(-35, -35, 70, 70), 0, 90*16 )
        painter.rotate( - @currentAngle )
        painter.drawRect( @barrelRect )
        painter.restore
    end

    private :paintShot, :paintTarget, :paintBarrier, :paintCannon

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
        r.moveCenter( Qt5::QPoint.new(@target.x(),height() - 1 - @target.y()) )
        return r
    end
    
    def barrierRect()
        return Qt5::QRect.new( 145, height() - 100, 15, 99 )
    end

    def barrelHit( pos )
        matrix = Qt5::QMatrix.new
        matrix.translate( 0, height() )
        matrix.rotate( - @currentAngle )
        matrix = matrix.inverted()
        return @barrelRect.contains( matrix.map(pos) )
    end

    private :cannonRect, :shotRect, :targetRect, :barrierRect, :barrelHit

    def shooting?
        #return @autoShootTimer.active?
        return @autoShootTimer.isActive()
    end
end
