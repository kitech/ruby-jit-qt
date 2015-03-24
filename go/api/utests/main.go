package main;

import (
	"qt"
	
)

func main() {
	s := qt.NewString()
	s.Length()

	_ = qt.QString_Number(123)

	qt.QMax(1, 2)
	
	println(s)
}
