package eg

import (
	"unsafe"
	// "reflect"
)

// import "qgc"
import "qt/dynamic"

type QString struct {
	Xa unsafe.Pointer
	Xb dynamic.QClass

	// AStaticMethod func(args... interface{}) interface{}
}

// 这种常量可能在编译时就替换成实际值了，运行时取不到相关常量信息
type EQString int

const KeepEmptyParts EQString = 0

// class Enum method, use like this: eg.QString.KeepEmptyparts;
// 在什么时间执行求值调用呢？
// 当作参数的时候没有问题，可以在处理参数的时候求值。
// 其他给变量的时候不好处理。
func (rthis QString) KeepEmptyParts() int {
	return dynamic.SingletonConstMissing()
}

// qt global Enum method, use like this: eg.AlignLeft
func AlignLeft() int {
	return dynamic.QtConstMissing()
}

// 3种可能的静态方法调用封装API
// Static method, use liks this: eg.QString.StaticMethod(eg.QString{}, ...);
func (QString) AStaticMethod(args ...interface{}) interface{} {
	return dynamic.SingletonMethodMissing(nil, args...)
	return 0
}
func (rthis *QString) AStaticMethod2(args ...interface{}) interface{} {
	return dynamic.SingletonMethodMissing(rthis, args...)
	return 0
}
func QString__AStaticMethod(args ...interface{}) interface{} {
	return dynamic.SingletonMethodMissing(nil, args...)
}
func QString_AStaticMethod(args ...interface{}) interface{} {
	return dynamic.SingletonMethodMissing(nil, args...)
}

func QMax(args ...interface{}) interface{} {
	return dynamic.QtFunctionMissing(args...)
}

func NewString(args ...interface{}) *QString {
	x := dynamic.Initialize(args...)
	rv := new(QString)
	rv.Xa = x
	rv.Xb.X = x
	rv.Xb.A = 123
	rv.Xb.N = "QString"
	return rv
}

func (this *QString) Free() {
	dynamic.Destructor(this.Xa)
}

func (this *QString) Length(args ...interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

func (this *QString) AnyCall(args ...interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

/*
	func (this *QString) KeepEmptyParts() {
}
*/

////////////
type QApplication struct {
	Xa unsafe.Pointer
}

func NewApplication(args ...interface{}) *QApplication {
	x := dynamic.Initialize(args...)
	rv := new(QApplication)
	rv.Xa = x
	return rv
}

func (this *QApplication) Free() {
	dynamic.Destructor(this.Xa)
}

func (this *QApplication) Length(args ...interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

func (this *QApplication) Exec(args ...interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

func (this *QApplication) AnyCall(args ...interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

////////////
type QWidget struct {
	X unsafe.Pointer
}

func NewWidget(args ...interface{}) *QWidget {
	x := dynamic.Initialize(args...)
	rv := new(QWidget)
	rv.X = x
	return rv
}

func (this *QWidget) Free() {
	dynamic.Destructor(this.X)
}

func (this *QWidget) Length(args ...interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

func (this *QWidget) Show(args ...interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

func (this *QWidget) AnyCall(args ...interface{}) interface{} {
	return dynamic.MethodMissing(this, args...)
}

//////////普通函数
func QMax2() {

}
