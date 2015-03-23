package main;

import (
	"fmt"
	// "qt"
	"eg"
	"reflect"
)

func main() {
	// var sp *eg.QString;
	var sr eg.QString;
	// a := eg.QString.KeepEmptyParts(*sp)
	a2 := eg.QString.KeepEmptyParts(sr)
	
	b := eg.QString.KeepEmptyParts
	fmt.Println( a2, b, reflect.TypeOf(b))

	r := b(sr)

	var argv []reflect.Value = make([]reflect.Value, 1);
	argv[0] = reflect.New(reflect.TypeOf(sr)).Elem()
	dr := reflect.ValueOf(b).Call(argv)
	
	fmt.Println(reflect.TypeOf(1), r, dr, dr[0].Int())
	fmt.Println(reflect.TypeOf(eg.KeepEmptyParts))

	// static
	eg.QString.AStaticMethod(eg.QString{}, 1, 2)
	sr.AStaticMethod()
	eg.QString__AStaticMethod(1, 2);
}
