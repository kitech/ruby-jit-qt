package qgc

import (
	"log"
	"bytes"
	"unsafe"
	"go/ast"
	"go/token"
	"os"
	"runtime"
	"strings"
	"fmt"
)

// 俩工具
func G_USED(vars ...interface{}) {}
var glog = log.New(os.Stdout, "", log.LstdFlags | log.Lshortfile)
type gvoid* interface{}
type cvoid* unsafe.Pointer

func Log(args... interface{}) {
	var pc = make([]uintptr, 5)
	n := runtime.Callers(0, pc)
	G_USED(n)

	f := runtime.FuncForPC(pc[2])
	file, line := f.FileLine(pc[2])
	elems := strings.Split(file, "/")
	G_USED(elems, ":", line)
	
	fmt.Print("    /  /     :  :   ", elems[len(elems)-1], ":", line, " ", f.Name(), ": ")
	fmt.Println(args...)
	// println(f.Name(), elems[len(elems)-1], ":", line, " ")
}

/*
需要支持类似C++的重载
根据传递的参数，生成简单的mangle名字
*/
func MangleSuffixByCallArgs(args ...interface{}) string {
	var buf bytes.Buffer
	buf.WriteString("o")
	
	for idx, arg := range args {
		switch arg.(type) {
		case int:
			buf.WriteString("i")
		case bool:
			buf.WriteString("b")
		case string:
			buf.WriteString("s")
		case unsafe.Pointer:
			buf.WriteString("p")
		default:
			G_USED(idx)
			panic("unimpl argument type:")
		}
	}

	return buf.String()
}


func MangleSuffixByAstArgs(args []ast.Expr) string {
	var buf bytes.Buffer
	buf.WriteString("o")

	for idx, arg := range args {
		switch sda := arg.(type) {
		case *ast.BasicLit:
			switch sda.Kind {
			case token.STRING:
				buf.WriteString("s")
			case token.INT:
				buf.WriteString("i")
			default:
			}
		default: G_USED(idx)
			glog.Println(sda)
		}

	}
	
	return buf.String()
}
