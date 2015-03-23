package eg;

import (
	"unsafe"
)
// import "qgc"
import "dynamic"


type QString struct {	
	X unsafe.Pointer
	b dynamic.QClass
}

func NewString(args... interface{}) *QString {
	x := dynamic.Initialize(args...)
	rv := new(QString)
	rv.X = x
	rv.b.X = x
	rv.b.A = 123
	return rv
}

func (this *QString) Free() {
	dynamic.Destructor(this.X)
}

func (this *QString) Length(args... interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

func (this *QString) AnyCall(args... interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}


////////////
type QApplication struct {
	X unsafe.Pointer
}

func NewApplication(args... interface{}) *QApplication {
	x := dynamic.Initialize(args...)
	rv := new(QApplication)
	rv.X = x
	return rv
}

func (this *QApplication) Free() {
	dynamic.Destructor(this.X)
}

func (this *QApplication) Length(args... interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

func (this *QApplication) Exec(args... interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

func (this *QApplication) AnyCall(args... interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

////////////
type QWidget struct {
	X unsafe.Pointer
}

func NewWidget(args... interface{}) *QWidget {
	x := dynamic.Initialize(args...)
	rv := new(QWidget)
	rv.X = x
	return rv
}

func (this *QWidget) Free() {
	dynamic.Destructor(this.X)
}

func (this *QWidget) Length(args... interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

func (this *QWidget) Show(args... interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

func (this *QWidget) AnyCall(args... interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}
