package eg;

import (
	"unsafe"
)
import "qgc"

type QString struct {
	x unsafe.Pointer
}

func NewString(args... interface{}) *QString {
	x := qgc.Initialize(args...)
	rv := new(QString)
	rv.x = x
	qgc.Log(x)
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



