package main;

import (
	"fmt"
	// "qt"
	"eg"
	// "xgo"
)

type MyString struct {
	s string
}

func (this *MyString) Append(s string) {
}

func test_string() {
	// var s *qt.QBaseType
	s := eg.NewString("aaaaaa")
	// s.Append("aaaaaa")
	s.Length()
	fmt.Println(s)

	s2 := eg.NewString("bbbbbb")
	s2.Length()

	// var s3 qt.QBaseType
	// s3.Length()
	
}

// QMC = Qt method call
// QFC = Qt function call
func main() {
	// qt.KeepPkg()
	// xgo.KeepPkg()

	var gs MyString
	gs.Append("ccccc")

	a := eg.NewApplication(0, nil)

	w := eg.NewWidget()
	w.Show()
	
	a.Exec()
	
	fmt.Println("Hello World!!!");
}
