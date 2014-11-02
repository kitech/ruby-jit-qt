a = File.absolute_path(File.dirname(__FILE__))
ret = require "#{a}/test_init.rb"

module Qt5
  class RubyThreadFix
    # slots 'callback_timeout()'

    def initialize
      super()
      # Enable threading
      @callback_timer = Qt5::QTimer.new()
      # connect(@callback_timer, SIGNAL('timeout()'), SLOT('callback_timeout()'))
      Qt5::connectrb(@callback_timer, 'timeout()', self, :callback_timeout);
      @callback_timer.start(800)
    end

    def callback_timeout
      # puts 'aaaaaaaaaaaaaaaaaa'
      return nil;
      if !@@queue.empty?
        # Start a backup timer in case this one goes modal.
        @callback_timer2.start(100)
        @@queue.pop.call until @@queue.empty?
        # Cancel the backup timer
        @callback_timer2.stop if @callback_timer2
      end
    end
  end

end # module Qt
