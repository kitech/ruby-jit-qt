package qgc;

// qgc工具的基础封装实现


import (
	"log"
	"bytes"
	"strings"
	"go/ast"
	"go/parser"
	"go/printer"
	"go/token"
	"reflect"
	"text/template"
	"strconv"
	"io/ioutil"
	"os"

	"dynamic"
)

// 俩工具
func G_USED(vars ...interface{}) {}
var glog = log.New(os.Stdout, "", log.LstdFlags | log.Lshortfile)

////////////////
type QtCalls struct {
	klass string
	mths map[string]*ast.CallExpr // method_name => CallExpr
	ces map[*ast.CallExpr]string  // CallExpr => method_name
}

func NewQtCalls(klass_name string) *QtCalls {
	v := new(QtCalls)

	v.klass = klass_name
	v.mths = make(map[string]*ast.CallExpr)
	v.ces = make(map[*ast.CallExpr]string)
	
	return v
}

type MyVisitor struct {
	fset *token.FileSet
	goc bytes.Buffer
	cc bytes.Buffer
	hc bytes.Buffer
	cg CodeGen
	qtclasses map[string]*QtCalls // 所有用到的类记录，防止生成重复的类构造函数
}

func NewMyVisitor() *MyVisitor {
	v := new(MyVisitor)
	v.qtclasses = make(map[string]*QtCalls)

	return v
}

func (this *MyVisitor) Visit(n ast.Node) (w ast.Visitor) {
	if n == nil {
		return nil
	}
	
	t := reflect.TypeOf(n)
	// glog.Println(t, t.String(), n)

	switch ge := n.(type) {
	case *ast.CallExpr:
		glog.Println("it should be *ast.CallExpr:", ge)
		this.processCallee(ge)
	default:
		// glog.Println("unknown node type:", t)
	}

	if false && t.String() == "*ast.SelectorExpr" {
		e := n.(*ast.SelectorExpr)
		var buf bytes.Buffer
		printer.Fprint(&buf, this.fset, e)
		glog.Println(t, n, e.Sel, e.Pos(), e.End(), e.X, buf.String(), "--")
		
		if strings.HasPrefix(buf.String(), "qt.New") {
			cname := strings.Join([]string{"Q", buf.String()[6:]}, "")
			glog.Println("cname:", cname, "===")

			
		}
	}

	if false && t.String() == "*ast.CallExpr" {
		// gocstr, hhstr, ccstr := this.genCallee(n.(*ast.CallExpr))
		gostr, hhstr, ccstr := this.genCallee(n.(*ast.CallExpr))

		gostr = "package qt;\n/*\n#include \"core.h\"\n*/\nimport \"C\";\nimport \"unsafe\";\n\n" + gostr
		
		glog.Println(len(hhstr))
		if len(hhstr) > 0 {
			// TODO 可以把这个封装到单独的方法中，叫生成封装源代码文件生成方法。
			// 为了让go build自动扫描时不包含这些自动生成的文件，又额外添加了.inc扩展名。
			d, _ := ioutil.ReadAll(strings.NewReader(hhstr))
			ioutil.WriteFile("src/qt/core_auto.h.inc", d, os.ModePerm)
			d, _ = ioutil.ReadAll(strings.NewReader(ccstr))
			ioutil.WriteFile("src/qt/core_auto.cpp.inc", d, os.ModePerm)

			d, _ = ioutil.ReadAll(strings.NewReader(gostr))
			ioutil.WriteFile("src/qt/qt_auto.go", d, os.ModePerm)
		}
	}
	
	return this
}

func (this *MyVisitor) genExtHeaderFile() {

}

func (this *MyVisitor) genCppImplFile() {

}

func (this *MyVisitor) genGoWrapFile() {

}

// 为qt类的构造函数生成对应的go/cgo/c++代码
func (this *MyVisitor) genStructor(klass string, ce *ast.CallExpr) (string, string, string) {
	// 包括生成一个struct
	glog.Println(klass)

	// template code buffers
	var gocbuf bytes.Buffer
	var ccbuf bytes.Buffer
	var hhbuf bytes.Buffer

	// tempates
	tpl, err := template.ParseFiles("data/qt_auto.go.tpl")
	glog.Println(tpl, err)
	hhtpl, err := template.ParseFiles("data/core_auto_ctor.h.tpl")
	// glog.Println(hhtpl, err)
	cctpl, err := template.ParseFiles("data/core_auto_ctor.cpp.tpl")

	// 生成参数原型列表
	var ptlst [10]string
	var pplst [10]string
	var pclst [10]string
	var pglst [10]string
	var pwlst [10]string
	
	for idx, arg := range ce.Args {
		glog.Println(idx, arg, reflect.TypeOf(arg))
		ptlst[idx] = this.genParamTypeList(idx, arg)
		pplst[idx] = this.genParamPassList(idx, arg)
		pclst[idx] = this.genParamConv(idx, arg)
		pglst[idx] = this.genParamInGo(idx, arg)
		pwlst[idx] = this.genParamWrapInGo(idx, arg)
	}
	ptlstr := strings.Join(ptlst[0:len(ce.Args)], ", ")
	pplstr := strings.Join(pplst[0:len(ce.Args)], ", ")
	pclstr := strings.Join(pclst[0:len(ce.Args)], ", ")
	pglstr := strings.Join(pglst[0:len(ce.Args)], ", ")
	pwlstr := strings.Join(pwlst[0:len(ce.Args)], ", ")
	glog.Println("--", pclstr, "--", pglstr, "--", pwlstr)
		
	tpl.Execute(&gocbuf, struct {
		Klass string
		ParamConvs string
		ParamInGo string
		ParamWrapInGo string
	} {klass, pclstr, pglstr, pwlstr})

	hhtpl.Execute(&hhbuf, struct {
		Klass string
		ParamList string
	} {klass, ptlstr})

	cctpl.Execute(&ccbuf, struct {
		Klass string
		ParamList string
		ParamPassList string
	} {klass, ptlstr, pplstr})
		
	glog.Println(gocbuf.String(), hhbuf.String(), ccbuf.String())
	return gocbuf.String(), hhbuf.String(), ccbuf.String()
}

// 生成go封装的mangled方法，生成c++封装的mangled方法, 生成extern
func (this *MyVisitor) genMethod(klass, method string, ce *ast.CallExpr) (string, string, string) {
	// 
	glog.Println(klass)

	// template code buffers
	var gocbuf bytes.Buffer
	var ccbuf bytes.Buffer
	var hhbuf bytes.Buffer

	// tempates
	tpl, err := template.ParseFiles("data/core_auto_mth.go.tpl")
	glog.Println(tpl, err)
	hhtpl, err := template.ParseFiles("data/core_auto_mth.h.tpl")
	// glog.Println(hhtpl, err)
	cctpl, err := template.ParseFiles("data/core_auto_mth.cpp.tpl")
	
	// 生成参数原型列表
	var ptlst [10]string
	var pplst [10]string
	var pclst [10]string
	var pglst [10]string
	var pwlst [10]string
	
	for idx, arg := range ce.Args {
		glog.Println(idx, arg, reflect.TypeOf(arg))
		ptlst[idx] = this.genParamTypeList(idx, arg)
		pplst[idx] = this.genParamPassList(idx, arg)
		pclst[idx] = this.genParamConv(idx, arg)
		pglst[idx] = this.genParamInGo(idx, arg)
		pwlst[idx] = this.genParamWrapInGo(idx, arg)
	}
	ptlstr := strings.Join(ptlst[0:len(ce.Args)], ", ")
	pplstr := strings.Join(pplst[0:len(ce.Args)], ", ")
	pclstr := strings.Join(pclst[0:len(ce.Args)], ", ")
	pglstr := strings.Join(pglst[0:len(ce.Args)], ", ")
	pwlstr := strings.Join(pwlst[0:len(ce.Args)], ", ")
	eptlstr := ptlstr
	if len(ptlstr) > 0 { eptlstr = ", " + ptlstr }
	epplstr := pplstr
	if len(pplstr) > 0 { epplstr = ", " + pplstr }
	glog.Println("--", ptlstr, "--", pplstr, "--", pclstr, "--", pglstr, "--", pwlstr)

	// mangle操作
	mgSuffix := dynamic.MangleSuffixByAstArgs(ce.Args)
	G_USED(mgSuffix)
	glog.Println(mgSuffix)
	
	cmethod := strings.ToLower(method[0:1]) + method[1:]	
	
	tpl.Execute(&gocbuf, struct {
		Klass string
		Method string
		CMethod string
		ParamConvs string
		ParamInGo string
		ParamWrapInGo string
		MangleSuffix string
	} {klass, method, cmethod, epplstr, /*pclstr,*/ pglstr, pwlstr, mgSuffix})

	hhtpl.Execute(&hhbuf, struct {
		Klass string
		Method string
		CMethod string
		MangleSuffix string	
		ParamList string
	} {klass, method, cmethod, mgSuffix, eptlstr})

	cctpl.Execute(&ccbuf, struct {
		Klass string
		Method string
		CMethod string
		MangleSuffix string
		ParamList string
		ParamPassList string
	} {klass, method, cmethod, mgSuffix, eptlstr, pplstr})
		
	glog.Println(gocbuf.String(), hhbuf.String(), ccbuf.String())
	return gocbuf.String(), hhbuf.String(), ccbuf.String()
}

// 
func (this *MyVisitor) genMethodWrap(klass, method string) string {
	// template code buffers
	var gocbuf bytes.Buffer

	// tempates
	tpl, err := template.ParseFiles("data/core_auto_mth0.go.tpl")
	G_USED(err)

	cmethod := strings.ToLower(method[0:1]) + method[1:]
	tpl.Execute(&gocbuf, struct {
		Klass string
		Method string
		CMethod string
	} {klass, method, cmethod})

	return gocbuf.String()
}

func (this *MyVisitor) processCallee(ce *ast.CallExpr) {

	// mths, found := this.qtclasses[klass]
	if this.isQtStructorCall(ce) {
		glog.Println("structorrrrrrrrr:", ce.Fun)
		fn := ce.Fun.(*ast.SelectorExpr)
		klass := this.getSelectorString(fn)[6:]
		mths, found := this.qtclasses[klass]
		if found {
			mths.klass = klass
		} else {
			mth := NewQtCalls(klass)
			mth.mths[klass] = ce // ctor
			this.qtclasses[klass] = mth
		}
	} else if this.isQtMethodCall(ce) {
		glog.Println("methoddddddddddd:", ce.Fun)		
		fn := ce.Fun.(*ast.SelectorExpr)
		mthname := strings.Split(this.getSelectorString(fn), ".")[1]
		klass := this.getKlassName(fn)
		glog.Println("method.......:", klass, mthname)
		calls, found := this.qtclasses[klass]
		if found {
			calls.klass = klass
			mth, mthfound := calls.mths[mthname]
			if mthfound {
				glog.Println("===?", mth == ce)
			} else {
				glog.Println("===?", mth == ce)
				calls.mths[mthname] = ce
				calls.ces[ce] = mthname
			}
		} else {
			panic("vvvvvvvv") // 一定有类的记录了
		}
	} else {
		glog.Println("unknown call type:", ce.Fun)
	}
}

// TODO 可以拆分成3部分，生成extern代码，生成c++封装代码，生成go封装代码。
func (this *MyVisitor) genCallee(ce *ast.CallExpr) (string, string, string) {
	glog.Println("[", ce.Fun, ce.Args, len(ce.Args), "]")
	var gocbuf bytes.Buffer
	var ccbuf bytes.Buffer
	var hhbuf bytes.Buffer
	// var restr string
	
	if false {
		fty := reflect.TypeOf(ce.Fun)
		glog.Println(fty)
	}
	fn := ce.Fun.(*ast.SelectorExpr)
	glog.Println("fn.name:", fn.Sel.Name)
	
	var buf bytes.Buffer
	printer.Fprint(&buf, this.fset, fn)

	if strings.HasPrefix(buf.String(), "qt.New") {
		scname := buf.String()[6:] // short cname
		cname := strings.Join([]string{"Q", buf.String()[6:]}, "")
		glog.Println("cname:", cname, "===")
		t, _ := template.New("foo").Parse(
			"void *New{{.Klass}}({{.ParamList}}) " +
				" { return new {{.Klass}}({{.ParamPassList}}); }\n" +
				" void Free{{.Klass}}(void *p) { delete ({{.Klass}}*)p; } \n")
		t3, _ := template.New("foo3").Parse(
			"extern void *New{{.Klass}}({{.ParamList}});\n" +
			"extern void Free{{.Klass}}(void *p);\n" +
			"")
		t2, _ := template.New("foo2").Parse(
			"func Free{{.SKlass}}(this *QBaseType) { if this.fn != nil { C.Free{{.Klass}} } } \n" +
				"func New{{.SKlass}}({{.ParamInGo}}) *QBaseType" +
				" {\n {{.ParamWrapInGo}}\n" +
				" var o = C.New{{.Klass}}({{.ParamConvs}}); " +
				"\n " +
				" var c = \"{{.Klass}}\";\n" +
				" return &QBaseType{o, false, c, Free{{.SKlass}} }; }\n" +
				"")
		// t.Execute(&gocbuf, struct {Klass string} {cname})

		// 生成参数原型列表
		var ptlst [10]string
		var pplst [10]string
		var pclst [10]string
		var pglst [10]string
		var pwlst [10]string
		
		for i, arg := range ce.Args {
			glog.Println(i, arg, reflect.TypeOf(arg))
			ptlst[i] = this.genParamTypeList(i, arg)
			pplst[i] = this.genParamPassList(i, arg)
			pclst[i] = this.genParamConv(i, arg)
			pglst[i] = this.genParamInGo(i, arg)
			pwlst[i] = this.genParamWrapInGo(i, arg)
		}
		ptlstr := strings.Join(ptlst[0:len(ce.Args)], ", ")
		pplstr := strings.Join(pplst[0:len(ce.Args)], ", ")
		pclstr := strings.Join(pclst[0:len(ce.Args)], ", ")
		pglstr := strings.Join(pglst[0:len(ce.Args)], ", ")
		pwlstr := strings.Join(pwlst[0:len(ce.Args)], ", ")
		glog.Println(strings.Join(ptlst[0:len(ce.Args)], ", "), "--", pplstr,
			"--", pclstr, "--", pglstr, "--", pwlstr)
		
		t.Execute(&ccbuf, struct {
			Klass string
			ParamList string
			ParamPassList string
		} {cname, ptlstr, pplstr})

		t3.Execute(&hhbuf, struct {
			Klass string
			ParamList string
		} {cname, ptlstr} )

		t2.Execute(&gocbuf, struct {
			SKlass string
			Klass string
			ParamConvs string
			ParamInGo string
			ParamWrapInGo string
		} {scname, cname, pclstr, pglstr, pwlstr})

	} else {
		// 需要推断出是否在调用QClass类实例方法
		// this.getVarType(fn.X)
		this.isQtVar(fn.X);
		/*
		ot := reflect.TypeOf(fn.X.(*ast.Ident).Obj);
		glog.Println(fn.X, ot, fn.X.(*ast.Ident).Obj)
		obj := fn.X.(*ast.Ident).Obj
		if obj != nil {
			od := obj.Decl
			rot := reflect.TypeOf(od)
			glog.Println(obj.Kind, obj, obj.Type, obj.Decl, obj.Data, obj.Name, rot)
			rod := od.(*ast.ValueSpec)
			glog.Println(rod, rod.Type)
			// glog.Println(len(rod.Rhs), rod.Rhs[0], rod.Lhs[0])
			// glog.Println(reflect.TypeOf(rod.Rhs[0].(*ast.CallExpr).Fun))
		}
        */
	}

	// Qt Method Call
	if strings.HasPrefix(fn.Sel.Name, "QMC_") {
		
	}

	// Qt Function Call
	if strings.HasPrefix(fn.Sel.Name, "QFC_") {
		
	}

	glog.Println(gocbuf.String(), ccbuf.String(), hhbuf.String())
	return gocbuf.String(), hhbuf.String(), ccbuf.String()
}

// 获取一个变量的类型信息
func (this *MyVisitor) getVarType(e ast.Expr) {
	t1 := reflect.TypeOf(e)
	id1 := e.(*ast.Ident)
	glog.Println(e,"123", t1, id1.Obj)
	// 对于对象来说，肯定是Obj不为nil的Ident
	// Obj为nil的Ident，是包名
	if id1.Obj != nil {
		o1 := id1.Obj
		glog.Println(o1.Kind, o1.Name, o1.Decl, o1.Data, o1.Type) // o1.Type = nil
	}
}

func (this *MyVisitor) getKlassName(e *ast.SelectorExpr) string {
	klass, ok := this.isQtVar(e.X)
	if ok {
		return klass
	}
	return ""
}

// 把SelectorExpr变成字符串表达形式，如qt.newQxxx
func (this *MyVisitor) getSelectorString(e *ast.SelectorExpr) string {
	// fn := ce.Fun.(*ast.SelectorExpr)
	fn := e
	glog.Println("fn.name:", fn.Sel.Name)
	
	var buf bytes.Buffer
	printer.Fprint(&buf, this.fset, fn)

	return buf.String()
}

func (this *MyVisitor) isQtStructorCall(ce *ast.CallExpr) bool {
	fn := ce.Fun.(*ast.SelectorExpr)
	glog.Println("fn.name:", fn.Sel.Name)
	
	var buf bytes.Buffer
	printer.Fprint(&buf, this.fset, fn)

	if strings.HasPrefix(buf.String(), "qt.New") {
		return true
	}
	return false
}

func (this *MyVisitor) isQtMethodCall(ce *ast.CallExpr) bool {
	fn := ce.Fun.(*ast.SelectorExpr)
	glog.Println("fn.name:", fn.Sel.Name)
	
	var buf bytes.Buffer
	printer.Fprint(&buf, this.fset, fn)

	if strings.HasPrefix(buf.String(), "qt.New") {
		return true
	} else {
		_, ok := this.isQtVar(fn.X)
		if ok {
			return true
		}
	}

	return false
}

// 判断一个变量是否是qt类对象
// 依据，这个变量的定义处，通过分析定义语句实现该判断。
func (this *MyVisitor) isQtVar(e ast.Expr) (string, bool) {
	t1 := reflect.TypeOf(e)
	id1 := e.(*ast.Ident)
	glog.Println(e, "123", t1, id1.Obj)

	// 对于对象来说，肯定是Obj不为nil的Ident
	// Obj为nil的Ident，是包名
	if id1.Obj == nil {
		glog.Println("maybe it is package:", e, ", but not possible a var")
		return "", false;
	}
	
	o1 := id1.Obj
	glog.Println(o1.Kind, o1.Name, o1.Decl, o1.Data, o1.Type) // o1.Type = nil
	t2 := reflect.TypeOf(o1.Decl)
	glog.Println(t2, t2.String(), t2.Kind());

	// 如果是ValueSpec
	// 如果是*ast.AssignStmt
	switch o1.Decl.(type) {
	case *ast.AssignStmt:
		glog.Println("assssssssssment...")
		o2 := o1.Decl.(*ast.AssignStmt)
		glog.Println(o2, o2.Rhs, o2.Rhs[0])
		t3 := reflect.TypeOf(o2.Rhs[0])
		fn := o2.Rhs[0].(*ast.CallExpr).Fun.(*ast.SelectorExpr)
		ss := this.getSelectorString(fn)
		glog.Println(t3, this.getSelectorString(fn))
		if strings.HasPrefix(ss, "qt.New") {
			glog.Println("ok, it is qt var:", e);
			return ss[6:], true
		}

	case *ast.ValueSpec:
		glog.Println("value specccccccc...")
		vsv := o1.Decl.(*ast.ValueSpec)
		glog.Println(vsv, "--", vsv.Names, "--", vsv.Type, "--", vsv.Values)
		t3 := reflect.TypeOf(vsv.Type)
		// sev := vsv.Type.(*ast.Ident) or (*ast.StarExpr)??? 应该是Unresolved Type才解析不出来的吧。
		glog.Println(t3, vsv.Type.Pos(), vsv.Type.End(), vsv.Type.End() - vsv.Type.Pos())

		// 现在遇到有两种可能。
		switch vsv.Type.(type) {
		case *ast.Ident: glog.Println("identttttttttt", vsv.Type.(*ast.Ident).String())
			o4 := vsv.Type.(*ast.Ident)
			if o4.String() == "qt.QBaseType" {
				glog.Println("ok it is a qt var:", e)
				return "QBaseType", true
			}
		case *ast.StarExpr: glog.Println("starrrrrrrrrr", vsv.Type.(*ast.StarExpr))
			o4 := vsv.Type.(*ast.StarExpr)
			o5 := this.getSelectorString(o4.X.(*ast.SelectorExpr))
			glog.Println(o4, o4.X, o4.Star, reflect.TypeOf(o4.X), o5)
			if o5 == "qt.QBaseType" {
				glog.Println("ok it is a qt var:", e)
				return "QBaseType", true				
			}
		case *ast.SelectorExpr:
			o4 := vsv.Type.(*ast.SelectorExpr)
			o5 := this.getSelectorString(o4)
			if o5 == "qt.QBaseType" {
				glog.Println("ok it is a qt var:", e, ", but not a pointer")
			}
		default: glog.Println("hehhhhhhhhhhhhhhhhhhh", vsv.Type)
		}
		
	default: panic("hehhhhhhhhhh")
	}


	return "", false;
}

// 获取调用的参数的类型，带有参数名，
// 给C中的封装函数使用，如char *arg0 ...，或者int arg0 ...
func (this *MyVisitor) genParamTypeList(idx int, param interface{}) string {
	aty := reflect.TypeOf(param)
	var buf bytes.Buffer
	// var args []interface{}

	if idx > 0 {
		// buf.WriteString(", ")
	}
	
	if aty.String() == "*ast.BasicLit" {
		lit := param.(*ast.BasicLit)
		glog.Println(lit.Kind)
		switch lit.Kind {
		case token.INT:
			glog.Println("token string:", lit.Value)
			buf.WriteString("int arg")
			buf.WriteString(strconv.Itoa(idx))
		case token.STRING:
			glog.Println("token string:", lit.Value)
			buf.WriteString("char *arg")
			buf.WriteString(strconv.Itoa(idx))
			// args[0] = C.CString(lit.Value)
			// buf.WriteString("C.CString(")
			// buf.WriteString(lit.Value)
			// buf.WriteString(")")
		default:
			glog.Println("unknown token type:", lit.Kind)
		}
	}

	
	glog.Println("params:", buf.String())
	buf.WriteString("")
	return buf.String()
	return ""
}

// 获取参数转换代码，给在go封装函数中使用。如C.int(123)
func (this *MyVisitor) genParamConv(idx int, param interface{}) string {
	aty := reflect.TypeOf(param)
	var buf bytes.Buffer
	// var args []interface{}

	if idx > 0 {
		// buf.WriteString(", ")
	}
	
	if aty.String() == "*ast.BasicLit" {
		lit := param.(*ast.BasicLit)
		glog.Println(lit.Kind)
		switch lit.Kind {
		case token.INT:
			glog.Println("token string:", lit.Value)
			buf.WriteString("C.int(")
			buf.WriteString(lit.Value)
			buf.WriteString(")")
		case token.STRING:
			glog.Println("token string:", lit.Value)
			// args[0] = C.CString(lit.Value)
			buf.WriteString("C.CString(")
			buf.WriteString(lit.Value)
			buf.WriteString(")")
		default:
			glog.Println("unknown token type:", lit.Kind)
		}
	}

	
	glog.Println("params:", buf.String())
	buf.WriteString("")
	return buf.String()
}

// 获取go封装所需要的参数，如arg0 string，或者arg0 int
func (this *MyVisitor) genParamInGo(idx int, param interface{}) string {
	aty := reflect.TypeOf(param)
	var buf bytes.Buffer
	// var args []interface{}

	if idx > 0 {
		// buf.WriteString(", ")
	}
	
	if aty.String() == "*ast.BasicLit" {
		lit := param.(*ast.BasicLit)
		glog.Println(lit.Kind)
		switch lit.Kind {
		case token.INT:
			glog.Println("token string:", lit.Value)
			buf.WriteString("arg")
			buf.WriteString(strconv.Itoa(idx))
			buf.WriteString(" int")
		case token.STRING:
			glog.Println("token string:", lit.Value)
			buf.WriteString("arg")
			buf.WriteString(strconv.Itoa(idx))
			buf.WriteString(" string")
		default:
			glog.Println("unknown token type:", lit.Kind)
		}
	}
	
	glog.Println("params:", buf.String())
	buf.WriteString("")
	return buf.String()
}

// 获取在go封装方法中调用C封装函数的参数变换，针对一个参数的语句。
func (this *MyVisitor) genParamWrapInGo(idx int, param interface{}) string {
	aty := reflect.TypeOf(param)
	var buf bytes.Buffer
	// var args []interface{}

	if idx > 0 {
		// buf.WriteString(", ")
	}
	
	if aty.String() == "*ast.BasicLit" {
		lit := param.(*ast.BasicLit)
		glog.Println(lit.Kind)
		switch lit.Kind {
		case token.INT:
			glog.Println("token string:", lit.Value)
			buf.WriteString("var _arg")
			buf.WriteString(strconv.Itoa(idx))
			buf.WriteString(" = ")
			buf.WriteString("arg")
			buf.WriteString(strconv.Itoa(idx))
			buf.WriteString(";\n")
		case token.STRING:
			glog.Println("token string:", lit.Value)
			buf.WriteString("var _arg")
			buf.WriteString(strconv.Itoa(idx))
			buf.WriteString(" = ")
			buf.WriteString("C.CString(arg")
			buf.WriteString(strconv.Itoa(idx))
			buf.WriteString(");\n")
			buf.WriteString("defer C.free(unsafe.Pointer(_arg")
			buf.WriteString(strconv.Itoa(idx))
			buf.WriteString("));\n")
		default:
			glog.Println("unknown token type:", lit.Kind)
		}
	}
	
	glog.Println("params:", buf.String())
	buf.WriteString("")
	return buf.String()
}

// 生成参数名
func (this *MyVisitor) genParamPassList(idx int, param interface{}) string {
	var buf bytes.Buffer
	buf.WriteString("arg")
	buf.WriteString(strconv.Itoa(idx))
	return buf.String()
}

// 生成方法体内临时变量名。
func (this *MyVisitor) genParamPassInGo(idx int, param interface{}) string {
	var buf bytes.Buffer
	buf.WriteString("_arg")
	buf.WriteString(strconv.Itoa(idx))
	return buf.String()
}

// 保存生成的代码
func (this *MyVisitor) saveExtHeaderFile() {

}

func (this *MyVisitor) saveCppImplFile() {

}

func (this *MyVisitor) saveGoWrapFile() {

}

func (this *MyVisitor) saveGeneratedCodeFile() {
	hhstr := this.hc.String()
	d, _ := ioutil.ReadAll(strings.NewReader(hhstr))
	ioutil.WriteFile("src/qt/core_auto.h.inc", d, os.ModePerm)

	ccstr := this.cc.String()
	d, _ = ioutil.ReadAll(strings.NewReader(ccstr))
	ioutil.WriteFile("src/qt/core_auto.cpp.inc", d, os.ModePerm)

	gostr := this.goc.String()
	gostr = "package qt;\n/*\n#include \"core.h\"\n*/\nimport \"C\";\nimport \"unsafe\";\n\n" + gostr		
	d, _ = ioutil.ReadAll(strings.NewReader(gostr))
	ioutil.WriteFile("src/qt/qt_auto.go", d, os.ModePerm)	
}

// 处理所有收集到的qt类及qt类方法调用
func (this *MyVisitor) processCollects() {
	for klass, calls := range this.qtclasses {
		G_USED(klass, calls)
		// maybe gen some comment string here
		
		///// gen methods
		for mthname, ce := range calls.mths {
			G_USED(mthname, ce)
			// genctor
			if mthname == klass {
				gocstr, hhstr, ccstr := this.genStructor(klass, ce)
				this.goc.WriteString(gocstr)
				this.hc.WriteString(hhstr)
				this.cc.WriteString(ccstr)				
			} else { // gen mth
				gocstr := this.genMethodWrap(klass, mthname)
				this.goc.WriteString(gocstr)
				
				gocstr, hhstr, ccstr := this.genMethod(klass, mthname, ce)
				this.goc.WriteString(gocstr)
				this.hc.WriteString(hhstr)
				this.cc.WriteString(ccstr)
			}
		}
	}
}

//
var gv string

/*
基本算法设计：
遍历当前目录.以及src/下的子目录所有的go文件，
查找使用的qt变量及qt对象的方法调用。
生成临时代码，存放在一个目录供当前目录编译时使用。

生成的代码存储位置，
./src/qt/*.go
./src/qt/*.cpp
./src/qt/*.h
*/
func Main() {

	fset := token.NewFileSet()
	f, err := parser.ParseFile(fset, "main.go", nil, parser.AllErrors)
	q, err := parser.ParseFile(fset, "src/qt/qt.go", nil, parser.AllErrors)
	pkgs, err := parser.ParseDir(fset, ".", nil, parser.AllErrors)
	glog.Println(f, err, q, "===", fset)

	for _, s := range f.Unresolved {
		glog.Println(s, "--")
	}

	mv := NewMyVisitor()
	mv.fset = fset

	for n, pkg := range pkgs {
		glog.Println(n, "--", pkg, "++")
	}
	ast.Walk(mv, f)
	glog.Println(mv, mv.qtclasses, len(mv.qtclasses), mv.qtclasses["String"].mths)
	mv.processCollects()

	mv.saveGeneratedCodeFile()
}


