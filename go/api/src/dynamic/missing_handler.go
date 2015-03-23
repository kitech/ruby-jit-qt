package dynamic;

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
	"strings"
	"strconv"
	
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

	C.gx_Qt_class_new(1, unsafe.Pointer(gvs), gv.star)
	
	G_USED(gv)
}

func init() {
	C.Init_forgo()
}


func Initialize(args... interface{}) unsafe.Pointer {
	var finit func()()
	G_USED(finit)

	// 根据调用栈获取Qt类名
	var getClassName = func () string {
		var pc = make([]uintptr, 5)
		runtime.Callers(0, pc)
		fn := runtime.FuncForPC(pc[3])
		cname := "Q" + fn.Name()[6:] // qt.NewString => QString
		
		return cname
	}
	G_USED(getClassName)
	// 获取调用者返回值类型，可能取不到啊。
	var getReturnType = func () reflect.Type {
		var pc = make([]uintptr, 5)
		runtime.Callers(0, pc)
		fn := runtime.FuncForPC(pc[3])
		Log(fn.Name())
		rfn := reflect.TypeOf(fn)

		return rfn
	}
	G_USED(getReturnType)
	
	var gvs *C.GoVarArray = C.newGoVarArray(1)
	var gv *C.GoVar = C.newGoVar()
	gv.kind = C.Int32Ty
	C.setElem(gvs, 0, gv);

	klass := getClassName()
	cklass := C.CString(klass)
	defer C.free(unsafe.Pointer(cklass))

	Log("abc:", getReturnType())
	
	o := C.gx_Qt_class_new(1, unsafe.Pointer(gvs), unsafe.Pointer(cklass))
	Log(o, reflect.TypeOf(o))

	return o
	return nil
}

func Destructor(qthis unsafe.Pointer) {
	var fdtor func()()
	G_USED(fdtor)

}

func getCallerName() string {
	var pc = make([]uintptr, 5)
	runtime.Callers(0, pc)
	f := runtime.FuncForPC(pc[3])
	Log(pc, f.Name(), strings.Split(f.Name(), ".")[2])
	name := strings.Split(f.Name(), ".")[2] // eg.(*QApplication).Exec
	name =  strings.ToLower(name[0:1]) + name[1:]
	G_USED(strconv.Atoi("123"))
	
	return name
	return ""
}

// private
func getCurrentFunctionName() string {
	pc := make([]uintptr, 10)
	runtime.Callers(2, pc)
	f := runtime.FuncForPC(pc[0])
	//fmt.Println(f.Name())
	return f.Name()
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

	var getElemValue = func (this interface{}, name string) unsafe.Pointer {
		tv := reflect.ValueOf(this)
		return tv.Elem().FieldByName(name).Interface().(unsafe.Pointer)
	}
	G_USED(getElemValue)

	ty := reflect.TypeOf(this)
	tv := reflect.ValueOf(this)
	field, err := ty.Elem().FieldByName("X")
	// 结构体字段必须是大写字母开关，即exported字段才能在不同的包中反射出来。
	Log(this, field, err, tv.Elem().FieldByName("X").Interface().(unsafe.Pointer), getElemValue(this, "X"))

	// 根据调用栈获取Qt类名
	// Log(ty.Elem().Name());
	
	// arguments
	argc := 2 + len(args)
	var gvs *C.GoVarArray = C.newGoVarArray(C.int(argc))
	var gv_klass_name *C.GoVar = C.newGoVar()
	gv_klass_name.kind = C.StringTy
	gv_klass_name.str = C.CString(ty.Elem().Name());
	defer C.free(unsafe.Pointer(gv_klass_name.str))
	C.setElem(gvs, 0, gv_klass_name);
	
	var gv *C.GoVar = C.newGoVar()
	gv.kind = C.StringTy
	gv.str = C.CString(getCallerName());
	defer C.free(unsafe.Pointer(gv.str))
	C.setElem(gvs, 1, gv);

	// real arguments
	for idx, arg := range args {
		Log(idx, arg)
		ty2 := reflect.TypeOf(arg)
		tv2 := reflect.ValueOf(arg)
		G_USED(ty2, tv2)

		var gv2 *C.GoVar = C.newGoVar()
		switch val := arg.(type) {
		case bool:
			gv2.kind = C.BoolTy
			gv2.b = C.bool(val)
		case int32:
			gv2.kind = C.Int32Ty
			gv2.i32 = C.int(val)
		case float32:
			gv2.kind = C.Float32Ty
			gv2.f32 = C.float(val)
		case string:
			gv2.kind = C.StringTy;
			gv2.str = C.CString(val)
		case unsafe.Pointer:
			gv2.kind = C.UnsafePointerTy
			gv2.star = val
		default:
			panic("unkown type:" + ty2.String())
		}
		C.setElem(gvs, C.int(idx + 2), gv2)
	}

	rv := C.gx_Qt_class_method_missing(1, unsafe.Pointer(gvs), getElemValue(this, "X"))
	Log(rv, getCallerName())
	
	return 5
}

// 静态类方法调用
func SingletonMethodMissing(rthis interface{}, args... interface{}) interface{} {
	
	return 6;
}

// 常量求值
func ConstMissing() int {
	return 123;
}
