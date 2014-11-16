#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt'
require 'gamebrd.rb'
#require './gamebrd.rb'

app = Qt5::QApplication.new(ARGV.count, ARGV)
board = GameBoard.new
board.setGeometry( 100, 100, 500, 355 )
board.show
app.exec
