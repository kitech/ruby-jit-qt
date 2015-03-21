package qgc;

/*
// 可重复
#cgo CFLAGS: -Wall -O2
#cgo CFLAGS: -I/usr/include/ruby-2.2.0 -I/usr/include/ruby-2.2.0/x86_64-linux
#cgo CXXFLAGS: -x c++ -std=c++1y -Wall -O2 -fPIC -I/usr/include/qt -I/usr/include/qt/QtCore
#cgo CXXFLAGS: -I/home/gzleo/opensource/rubyjitqt/go
#cgo CXXFLAGS: -I/usr/include/ruby-2.2.0  -I/usr/include/ruby-2.2.0/x86_64-linux
#cgo LDFLAGS:  -Wl,-rpath,/home/gzleo/opensource/rubyjitqt/ -L/home/gzleo/opensource/rubyjitqt/ -lhandby
// #cgo LDFLAGS: -lstdc++ -lQt5Core

#include "qgc.h"
#include "vtype.h"

*/
import "C"
import "unsafe"
import (
	"runtime"
	"reflect"
	
	"gopkgs.com/dl.v1"
)

var dlh *dl.DL = nil
func getDl() *dl.DL {
	if dlh == nil {
		tdlh, err := dl.Open("", dl.RTLD_LAZY)
		if err != nil {
			panic("dlopen error:" + err.Error())
		} else {
			dlh = tdlh
		}
	}
	return dlh
}

func test() {
	G_USED(reflect.Uint)
	
	var gv *C.GoVar = C.newGoVar()
	gv.kind = C.Int32Ty
	gv.ukind = C.Uint32Ty
	
	gv.name = C.CString("name hehehhhhhh")
	gv.size = 567;
	// gv.goval = unsafe.Pointer(&reflect.TypeOf(gv).Elem())
	gv.i32 = 1
	gv.star = unsafe.Pointer(gv)

	var gvs *C.GoVarArray = C.newGoVarArray(5)
	C.setElem(gvs, 1, gv)
	
	G_USED(gv)
}

func init() {
	C.Init_forgo()
	Log("fffffffff")
}

func Initialize(args... interface{}) unsafe.Pointer {
	var finit func()()
	G_USED(finit)

	
	return nil
}

func Destructor(qthis unsafe.Pointer) {
	var fdtor func()()
	G_USED(fdtor)

}

func getCallerName() string {
	var pc = make([]uintptr, 5)
	runtime.Callers(0, pc)
	f := runtime.FuncForPC(pc[2])
	Log(pc, f.Name())
	
	return ""
}

/*
可能的返回值类型:
void
int
pointer
reference
object    
*/
func MethodMissing(this interface{}, args... interface{}) interface{} {
	var fmth func()()
	G_USED(fmth)
	
	return 5
}

