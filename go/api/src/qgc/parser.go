package qgc;

// qgc工具的基础封装实现


import (
	"bytes"
	"strings"
	"go/ast"
	"go/parser"
	"go/printer"
	"go/token"
	"reflect"
	"text/template"
	// "strconv"
	"time"
	"io/ioutil"
	"os"

	"dynamic"
)

type CodeGen struct {
	buf bytes.Buffer
	types map[string]int // klass -> bool
	mths map[string]int // klass + method -> bool
	consts map[string]int // klass + const -> bool
	funcs map[string]int // function -> bool
	gconsts map[string]int // const -> bool
}

func NewCodeGen() *CodeGen {
	o := new(CodeGen)
	
	o.types = make(map[string]int, 1)
	o.mths = make(map[string]int, 5)
	o.consts = make(map[string]int, 10)
	o.funcs = make(map[string]int, 10)
	o.gconsts = make(map[string]int, 5)

	o.genHeader();
	
	return o
	return nil
}

func (this *CodeGen) dump() {
	println(this.buf.String())
}

func (this *CodeGen) saveFile() {
	ccstr := this.buf.String()
	data, _ := ioutil.ReadAll(strings.NewReader(ccstr))
	ioutil.WriteFile("utests/src/qt/qt.go", data, os.ModePerm)
}

func (this *CodeGen) genHeader() string {
	t := time.Now().String()
	tpl := "// auto generated: " + t + "\n" + 
		"package qt;\n\n" +
		"import \"unsafe\";\n" +
		"import \"dynamic\";\n" +
		"\n"
		
	this.buf.WriteString(tpl)
	return ""
}

func (this *CodeGen) genType(klass string) string {
	tpl := "type Q{{.Klass}} struct {\n" +
		"    X unsafe.Pointer\n" +
		"    B dynamic.QClass\n" +
		"}\n\n"

	type header struct {
		Klass string		
	}
	var tbuf bytes.Buffer
	t, _ := template.New("header").Parse(tpl)
	t.Execute(&tbuf, header{klass})
	
	this.buf.WriteString(tbuf.String())

	dynamic.Log(klass, len(tbuf.String()))

	this.genCtor(klass);
	this.genDtor(klass);
	
	return ""
}

func (this *CodeGen) genCtor(klass string) string {
	tpl := "func New{{.Klass}}(args... interface{}) *Q{{.Klass}} {\n" +
	"  x := dynamic.Initialize(args...)\n" +
	"  rv := new(Q{{.Klass}})\n" +
	"  rv.X = x\n" +
	"  rv.B.X = x\n" +
	"  rv.B.A = 123\n" +
	"  return rv\n" +
	"}\n" +
	""

	type header struct {
		Klass string		
	}
	var tbuf bytes.Buffer
	t, _ := template.New("header").Parse(tpl)
	t.Execute(&tbuf, header{klass})

	this.buf.WriteString(tbuf.String())

	return ""
}

func (this *CodeGen) genDtor(klass string) string {
	tpl := "func (this *Q{{.Klass}}) Free() {\n" +
		"  dynamic.Destructor(this.X)\n" +
		"}\n" +
		""

	type header struct {
		Klass string		
	}
	var tbuf bytes.Buffer
	t, _ := template.New("header").Parse(tpl)
	t.Execute(&tbuf, header{klass})

	this.buf.WriteString(tbuf.String())
	
	return ""
}

func (this *CodeGen) genMethod(klass, mth string) string {
	tpl := "func (this *Q{{.Klass}}) {{.Method}}(args... interface{}) interface{} {\n" +
		"  return dynamic.MethodMissing(this, args...)\n" +
		"}\n" +
		""

	type header struct {
		Klass string
		Method string
	}
	var tbuf bytes.Buffer
	t, _ := template.New("header").Parse(tpl)
	t.Execute(&tbuf, header{klass, mth})

	this.buf.WriteString(tbuf.String())

	return ""
}

func (this *CodeGen) genStaticMethod(klass, mthname string) string {
	tpl := "func Q{{.Klass}}_{{.Method}}(args... interface{}) interface{} {\n" +
		"  return dynamic.SingletonMethodMissing(nil, args...);\n" +
		"}\n" +
		""
	
	type header struct {
		Klass string
		Method string
	}
	var tbuf bytes.Buffer
	t, _ := template.New("header").Parse(tpl)
	t.Execute(&tbuf, header{klass, mthname})

	this.buf.WriteString(tbuf.String())
	glog.Println(klass, mthname)

	return ""
}

func (this *CodeGen) genStaticMethod2(klass, mthname string) string {
	tpl := "func (rthis *Q{{.Klass}}) {{.Method}}2(args... interface{}) interface{} {\n" +
		"  return dynamic.SingletonMethodMissing(rthis, args...);\n" +
		"}\n" +
		""
	
	type header struct {
		Klass string
		Method string
	}
	var tbuf bytes.Buffer
	t, _ := template.New("header").Parse(tpl)
	t.Execute(&tbuf, header{klass, mthname})

	this.buf.WriteString(tbuf.String())
	glog.Println(klass, mthname)

	return ""
}

func (this *CodeGen) genStaticMethod3(klass, mthname string) string {
	tpl := "func (Q{{.Klass}}) {{.Method}}(args... interface{}) interface{} {\n" +
	"  return dynamic.SingletonMethodMissing(nil, args...);\n" +
	"}\n" +
	""
	
	type header struct {
		Klass string
		Method string
	}
	var tbuf bytes.Buffer
	t, _ := template.New("header").Parse(tpl)
	t.Execute(&tbuf, header{klass, mthname})

	this.buf.WriteString(tbuf.String())
	glog.Println(klass, mthname)

	return ""
}


func (this *CodeGen) genClassConstant(klass, ename string) string {
	tpl := "func (Q{{.Klass}}) {{.Name}}() int {\n" +
		"  return dynamic.SingletonConstMissing()\n" +
		"}\n" +
		""

	type header struct {
		Klass string
		Name string
	}
	var tbuf bytes.Buffer
	t, _ := template.New("header").Parse(tpl)
	t.Execute(&tbuf, header{klass, ename})

	this.buf.WriteString(tbuf.String())
	glog.Println(klass, ename)
	
	return ""
}


func (this *CodeGen) genFunction(fname string) string {

	tpl := "func {{.Name}}(args... interface{}) interface{} {\n" +
		"  return dynamic.QtFunctionMissing(args...);\n" +
		"}\n" +
		""
	
	type header struct {
		Name string
	}
	var tbuf bytes.Buffer
	t, _ := template.New("header").Parse(tpl)
	t.Execute(&tbuf, header{fname})

	this.buf.WriteString(tbuf.String())

	return ""
}

func (this *CodeGen) genGlobalConstant(ename string) string {
	tpl := "func {{.Name}}() int {\n" +
		"  return dynamic.QtConstMissing();\n" +
		"}\n" +
		""
	type header struct {
		Name string
	}
	var tbuf bytes.Buffer
	t, _ := template.New("header").Parse(tpl)
	t.Execute(&tbuf, header{ename})

	this.buf.WriteString(tbuf.String())
	glog.Println(ename)
	
	return ""
}


////////////////
type DynamicQtCalls struct {
	klass string
	mths map[string]*ast.CallExpr // method_name => CallExpr
	ces map[*ast.CallExpr]string  // CallExpr => method_name
}

func NewDynamicQtCalls(klass_name string) *DynamicQtCalls {
	v := new(DynamicQtCalls)

	v.klass = klass_name
	v.mths = make(map[string]*ast.CallExpr)
	v.ces = make(map[*ast.CallExpr]string)
	
	return v
}

// 实现生成dynamic方式绑定封装的Visitor
type DynamicVisitor struct {
	fset *token.FileSet
	goc bytes.Buffer
	cc bytes.Buffer
	hc bytes.Buffer
	cg *CodeGen
	qtclasses map[string]*DynamicQtCalls // 所有用到的类记录，防止生成重复的类构造函数
}

func NewDynamicVisitor() *DynamicVisitor {
	v := new(DynamicVisitor)
	v.qtclasses = make(map[string]*DynamicQtCalls)
	v.cg = NewCodeGen()

	return v
}

func (this *DynamicVisitor) Visit(n ast.Node) (w ast.Visitor) {
	if n == nil {
		return nil
	}
	
	// t := reflect.TypeOf(n)
	// glog.Println(t, t.String(), n)

	switch ge := n.(type) {
	case *ast.CallExpr:
		glog.Println("it should be *ast.CallExpr:", ge)
		this.processCallee(ge)
	default:
		// glog.Println("unknown node type:", t)
	}

	return this
}

func (this *DynamicVisitor) processCallee(ce *ast.CallExpr) {
	// like println() call
	switch ce.Fun.(type) {
	case *ast.Ident:
		return;
	}
	
	// mths, found := this.qtclasses[klass]
	if this.isQtStructorCall(ce) {
		glog.Println("structorrrrrrrrr:", ce.Fun)
		fn := ce.Fun.(*ast.SelectorExpr)
		klass := this.getSelectorString(fn)[6:]
		this.cg.genType(klass)
		
	} else if this.isQtMethodCall(ce) {
		glog.Println("methoddddddddddd:", ce.Fun)		
		fn := ce.Fun.(*ast.SelectorExpr)
		mthname := strings.Split(this.getSelectorString(fn), ".")[1]
		klass := this.getKlassName(fn)
		glog.Println("method.......:", klass, mthname)

		this.cg.genMethod(klass, mthname)
	} else if this.isStaticMethodCall(ce) {
		glog.Println("static methoddddddddddd:", ce.Fun)
		fn := ce.Fun.(*ast.SelectorExpr)
		mthname := strings.Split(this.getSelectorString(fn), "_")[1]
		klass := strings.Split(this.getSelectorString(fn), "_")[0][4:]
		glog.Println("static method.......:", klass, mthname)

		this.cg.genStaticMethod(klass, mthname)
		this.cg.genStaticMethod2(klass, mthname)
		this.cg.genStaticMethod3(klass, mthname)

	} else if this.isQtFunctionCall(ce) {
		glog.Println("qt functionnnnnnnnnnnn:", ce.Fun)
		fn := ce.Fun.(*ast.SelectorExpr)
		fname := strings.Split(this.getSelectorString(fn), ".")[1]
		glog.Println("function .......:", fname)

		this.cg.genFunction(fname)
	} else {
		glog.Println(this.isStaticMethodCall(ce))
		glog.Println(this.isQtFunctionCall(ce))
		glog.Println("unknown call type:", ce.Fun, reflect.TypeOf(ce.Fun))
	}
}

// 是否是静态方法调用
func (this *DynamicVisitor) isStaticMethodCall(ce *ast.CallExpr) bool {

	str := this.getSelectorString(ce.Fun.(*ast.SelectorExpr))
	// qt.QString_Number
	// using regex???

	if strings.HasPrefix(str, "qt.Q") && len(strings.Split(str, "_")) == 2 {
		return true
	}
	
	return false
}

func (this *DynamicVisitor) isQtFunctionCall(ce *ast.CallExpr) bool {
	str := this.getSelectorString(ce.Fun.(*ast.SelectorExpr))
	// qt.QMax
	// glog.Println(strings.Split(str, "_"))
	if strings.HasPrefix(str, "qt.Q") && len(strings.Split(str, "_")) == 1 {
		return true
	}
	
	return false
}


// 获取一个变量的类型信息
func (this *DynamicVisitor) getVarType(e ast.Expr) {
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

func (this *DynamicVisitor) getKlassName(e *ast.SelectorExpr) string {
	klass, ok := this.isQtVar(e.X)
	if ok {
		return klass
	}
	return ""
}

// 把SelectorExpr变成字符串表达形式，如qt.newQxxx
func (this *DynamicVisitor) getSelectorString(e *ast.SelectorExpr) string {
	// fn := ce.Fun.(*ast.SelectorExpr)
	fn := e
	glog.Println("fn.name:", fn.Sel.Name)
	
	var buf bytes.Buffer
	printer.Fprint(&buf, this.fset, fn)

	return buf.String()
}

func (this *DynamicVisitor) isQtStructorCall(ce *ast.CallExpr) bool {
	
	fn := ce.Fun.(*ast.SelectorExpr)
	glog.Println("fn.name:", fn.Sel.Name)
	
	var buf bytes.Buffer
	printer.Fprint(&buf, this.fset, fn)

	if strings.HasPrefix(buf.String(), "qt.New") {
		return true
	}
	return false
}

func (this *DynamicVisitor) isQtMethodCall(ce *ast.CallExpr) bool {
	
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
func (this *DynamicVisitor) isQtVar(e ast.Expr) (string, bool) {
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
func DynamicMain() {

	fset := token.NewFileSet()
	f, err := parser.ParseFile(fset, "utests/main.go", nil, parser.AllErrors)
	// q, err := parser.ParseFile(fset, "src/qt/qt.go", nil, parser.AllErrors)
	pkgs, err := parser.ParseDir(fset, ".", nil, parser.AllErrors)
	glog.Println(f, err, "===", fset)

	for _, s := range f.Unresolved {
		glog.Println(s, "--")
	}

	mv := NewDynamicVisitor()
	mv.fset = fset

	for n, pkg := range pkgs {
		glog.Println(n, "--", pkg, "++")
	}
	ast.Walk(mv, f)
	// glog.Println(mv, mv.qtclasses, len(mv.qtclasses), mv.qtclasses["String"].mths)

	mv.cg.dump()
	mv.cg.saveFile()
	// os.Exit(0)
}

