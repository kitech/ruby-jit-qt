package eg;

import (
	"unsafe"
)
import "qgc"


type QString struct {
	x unsafe.Pointer
	b qgc.QClass
}

func NewString(args... interface{}) *QString {
	x := qgc.Initialize(args...)
	rv := new(QString)
	rv.x = x
	rv.b.X = x
	rv.b.A = 123
	return rv
}

func (this *QString) Free() {
	qgc.Destructor(this.x)
}

func (this *QString) Length(args... interface{}) interface{} {
	return qgc.MethodMissing(this, args...)
}

func (this *QString) AnyCall(args... interface{}) interface{} {
	return qgc.MethodMissing(this, args...)
}


////////////
type QApplication struct {
	x unsafe.Pointer
}

func NewApplication(args... interface{}) *QApplication {
	x := qgc.Initialize(args...)
	rv := new(QApplication)
	rv.x = x
	return rv
}

func (this *QApplication) Free() {
	qgc.Destructor(this.x)
}

func (this *QApplication) Length(args... interface{}) interface{} {
	return qgc.MethodMissing(this, args...)
}

func (this *QApplication) Exec(args... interface{}) interface{} {
	return qgc.MethodMissing(this, args...)
}

func (this *QApplication) AnyCall(args... interface{}) interface{} {
	return qgc.MethodMissing(this, args...)
}

////////////
type QWidget struct {
	x unsafe.Pointer
}

func NewWidget(args... interface{}) *QWidget {
	x := qgc.Initialize(args...)
	rv := new(QWidget)
	rv.x = x
	return rv
}

func (this *QWidget) Free() {
	qgc.Destructor(this.x)
}

func (this *QWidget) Length(args... interface{}) interface{} {
	return qgc.MethodMissing(this, args...)
}

func (this *QWidget) Show(args... interface{}) interface{} {
	return qgc.MethodMissing(this, args...)
}

func (this *QWidget) AnyCall(args... interface{}) interface{} {
	return qgc.MethodMissing(this, args...)
}
