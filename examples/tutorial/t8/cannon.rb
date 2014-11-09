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
		p = Qt5::QPainter.new( self )
		p.drawText( 200, 200, "Angle = %d" % @currentAngle )
		p.end()
	end


	def sizePolicy()
    	return Qt5::QSizePolicy.new( Qt5::QSizePolicy::Expanding, Qt5::QSizePolicy::Expanding )
	end
end
