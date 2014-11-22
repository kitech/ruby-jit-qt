
#include "fix_clang_undef_eai.h"

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtNetwork>
#include <QtWebSockets>

#include <cassert>
#include <functional>
#include <typeinfo>


#include <llvm/ExecutionEngine/GenericValue.h>

#include "debugoutput.h"

#include "ruby.hpp"
#include "qtobjectmanager.h"

#include "utils.h"
#include "entry.h"

#include "clvm.h"

#include "metalize/metar_classes_qtcore.h"
#include "metalize/metas.h"
#include "qtruby.h"

// extern "C" {


/*
static VALUE x_QString_destructor(VALUE id)
{
    // qDebug()<<__FUNCTION__;

    VALUE os = rb_const_get(rb_cModule, rb_intern("ObjectSpace"));
    VALUE self = rb_funcall(os, rb_intern("_id2ref"), 1, id);
    // VALUE self = ID2SYM(id);
    GET_CI3(QString);
    qDebug()<<__FUNCTION__<<ci<<NUM2ULONG(id)<<QString("%1").arg(TYPE(self));
    delete ci;

    return Qnil;
}

static VALUE x_QString_init(VALUE self)
{
    yQString *s = NULL;
    s = new yQString();
    qDebug()<<s;

    // rb_iv_set(self, "@_ci", (VALUE)s);
    // Qom::inst()->objs[rb_to_id(self)] = (QObject*)s;
    SAVE_CI2(QString, s);
    // qDebug()<<TYPE(self);
    // rb_hash(self);

    VALUE free_proc = rb_proc_new((VALUE (*) (...)) x_QString_destructor, 0);
    rb_define_finalizer(self, free_proc);

    return self;
}

static QString * x_QString_to_q(VALUE self)
{
    GET_CI3(QString);
    return ci;
}

static VALUE x_QString_to_s(VALUE self)
{
    // QString *s = (QString*)rb_iv_get(self, "@_ci");
    GET_CI3(QString);

    QString to_s;
    QDebug out(&to_s);
    out << ci << "<--"<< ci->length();

    return (VALUE)rb_str_new2(to_s.toLatin1().data());
}

static VALUE x_QString_append(VALUE self, VALUE obj)
{
    // QString *ci = (QString*)rb_iv_get(self, "@_ci");
    GET_CI3(QString);
    
    if (TYPE(obj) == RUBY_T_STRING) {
        ci->append(RSTRING_PTR(obj));
    } else {
        qDebug()<<TYPE(obj);
        ci->append(RSTRING_PTR(x_QString_to_s(obj)));
    }

    return self;
}
*/

// #define VariantOrigValue(v)                         
#define VOValue(v)                            \
    (                                                  \
     (v.type() == QVariant::Invalid) ? QVariant() :    \
     (v.type() == QVariant::Bool) ? v.toBool() :       \
     (v.type() == QVariant::Int) ? v.toInt() :         \
     (v.type() == QVariant::Double) ? v.toDouble() :   \
     (v.type() == QVariant::Char) ? v.toChar() :       \
     (v.type() == QVariant::String) ? v.toString() :   \
     (v.type() == QVariant::Point) ? v.toPoint() :     \
     QVariant())


static void abc()
{
    QVariant v;

    // error, error: non-pointer operand type 'QPoint' incompatible with nullptr
    // 同一表达式的返回值类型应该相同啊。
    //*
    (
     (v.type() == QVariant::Invalid) ? QVariant() : 
     (v.type() == QVariant::Bool) ? v.toBool() :
     (v.type() == QVariant::Int) ? v.toInt() :
     (v.type() == QVariant::Double) ? v.toDouble() :
     (v.type() == QVariant::Char) ? v.toChar() :
     (v.type() == QVariant::String) ? v.toString() :
     (v.type() == QVariant::Point) ? v.toPoint() : QVariant()
     );
        // */
}

typedef struct {
    int iretval;
    bool bretval;
    QString sretval;
} ReturnStorage;

typedef struct {
    int iretval;
    bool bretval;
    QString sretval;

    // 最大支持10个参数，参数的临时值放在这
    int ival[10];
    bool bval[10];
    QString sval[10];
} InvokeStorage;

static QGenericReturnArgument makeRetArg(int type, ReturnStorage &rs)
{
    QGenericReturnArgument retval;

    switch (type) {
    case QMetaType::Int: retval = Q_RETURN_ARG(int, rs.iretval); break;
    case QMetaType::Bool: retval = Q_RETURN_ARG(bool, rs.bretval); break;
    case QMetaType::QString: retval = Q_RETURN_ARG(QString, rs.sretval); break;
    }

    return retval;
}

static VALUE retArg2Value(int type, QGenericReturnArgument retarg)
{
    VALUE retval = Qnil;
    ReturnStorage rs;

    switch (type) {
    case QMetaType::Int:
        rs.iretval = *((int*)retarg.data());
        retval = INT2NUM(rs.iretval);
        break;
    case QMetaType::QString:
        rs.sretval = *((QString*)retarg.data());
        retval = rb_str_new2(rs.sretval.toLatin1().data());
        break;
    case QMetaType::Bool:
        rs.bretval = *((bool*)retarg.data());
        retval = rs.bretval ? Qtrue : Qfalse;
        break;
    case QMetaType::Void:
        
        break;
    default:
        qDebug()<<"unknown ret type:"<<type<<retarg.name()<<retarg.data();
        break;
    };

    return retval;
}

static QGenericArgument Variant2Arg(int type, QVariant &v, int idx, InvokeStorage &is)
{
    QGenericArgument valx;

    switch (type) {
    case QMetaType::Int: 
        is.ival[idx] = v.toInt();
        valx = Q_ARG(int, is.ival[idx]);
        break;
    case QMetaType::QString: 
        is.sval[idx] = v.toString();
        valx = Q_ARG(QString, is.sval[idx]); 
        break;
    case QMetaType::Bool:
        is.bval[idx] = v.toBool();
        valx = Q_ARG(bool, is.bval[idx]);
        break;
    }

    return valx;
}

/*
  数字类型
  字符串类型
  对象类型
  数组类型(元素为数字，字符串，对象）
 */
static QVariant VALUE2Variant(VALUE v)
{

    QVariant rv;
    QString str, str2;
    void *ci = NULL;
    QObject *obj = NULL;

    VALUE v1;
    VALUE v2;
    VALUE v3;
    
    switch (TYPE(v)) {
    case T_NONE:  rv = QVariant(); break;
    case T_FIXNUM: rv = (int)FIX2INT(v); break;
    case T_STRING:  rv = RSTRING_PTR(v); break;
    case T_FLOAT:  rv = RFLOAT_VALUE(v); break;
    case T_NIL:   rv = 0; break;
    case T_TRUE:  rv = true; break;
    case T_FALSE: rv = false; break;
    case T_OBJECT:
        str = QString(rb_class2name(RBASIC_CLASS(v)));
        ci = Qom::inst()->jdobjs[v];
        // obj = dynamic_cast<QObject*>(ci);
        qDebug()<<"unimpl VALUE:"<<str<<ci<<obj;
        // rv = QVariant(QMetaType::VoidStar, ci);
        rv = QVariant::fromValue(ci);
        break;
    case T_ARRAY: {
        QStringList ary;
        // ary << "a123" << "b3456";
        qDebug()<<RARRAY_LEN(v)<<QT_VERSION;
        for (int i = 0; i < RARRAY_LEN(v); i++) {
            ary << VALUE2Variant(rb_ary_entry(v, i)).toString();
        }
        rv = QVariant(ary);
    }; break;
    case T_STRUCT:
        qDebug()<<"use VALUE2Variant2 function.";
        break;
    case T_CLASS:
    default:
        qDebug()<<"unknown VALUE type:"<<TYPE(v);
        break;
    }
    
    return rv;
}

// Range类型支持
static QVector<QVariant> VALUE2Variant2(VALUE v)
{
    QVector<QVariant> rvs(1);
    QVariant rv;
    QString str, str2;
    void *ci = NULL;
    QObject *obj = NULL;

    VALUE v1;
    VALUE v2;
    VALUE v3;
    
    switch (TYPE(v)) {
    case T_NONE:  rv = QVariant(); break;
    case T_FIXNUM: rv = (int)FIX2INT(v); break;
    case T_STRING:  rv = RSTRING_PTR(v); break;
    case T_FLOAT:  rv = RFLOAT_VALUE(v); break;
    case T_NIL:   rv = 0; break;
    case T_TRUE:  rv = true; break;
    case T_FALSE: rv = false; break;
    case T_OBJECT:
        str = QString(rb_class2name(RBASIC_CLASS(v)));
        ci = Qom::inst()->jdobjs[v];
        // obj = dynamic_cast<QObject*>(ci);
        qDebug()<<"unimpl VALUE:"<<str<<ci<<obj;
        // rv = QVariant(QMetaType::VoidStar, ci);
        rv = QVariant::fromValue(ci);
        break;
    case T_ARRAY: {
        QStringList ary;
        // ary << "a123" << "b3456";
        qDebug()<<RARRAY_LEN(v)<<QT_VERSION;
        for (int i = 0; i < RARRAY_LEN(v); i++) {
            ary << VALUE2Variant(rb_ary_entry(v, i)).toString();
        }
        rv = QVariant(ary);
    }; break;
    case T_STRUCT: {
        str = rb_class2name(RBASIC_CLASS(v));
        if (str == "Range") {
            // qDebug()<<"Range is struct???"<<BUILTIN_TYPE(v)
            //         <<rb_class2name(RBASIC_CLASS(v))
            //         <<RSTRUCT_LEN(v);
            v1 = RSTRUCT_GET(v, 0);
            v2 = RSTRUCT_GET(v, 1);
            v3 = RSTRUCT_GET(v, 2);
            // qDebug()<<TYPE(v1)<<TYPE(v2)<<TYPE(v3);
            // qDebug()<<FIX2INT(v1)<<FIX2INT(v2);

            rv = QVariant(FIX2INT(v1));
            rvs.append(QVariant(FIX2INT(v2)));
        } else {
            qDebug()<<"unsupported struct type:"<<str;
        }
    }; break;
    case T_CLASS:
    default:
        qDebug()<<"unknown VALUE type:"<<TYPE(v);
        break;
    }
    
    rvs[0] = rv;
    return rvs;
}

static QVector<QVariant> ARGV2Variant(int argc, VALUE *argv, int start = 0)
{
    QVector<QVariant> args;
    for (int i = start; i < argc; i ++) {
        // if (i == 0) continue; // for ctor 0 is arg0
        if (i >= argc) break;
        qDebug()<<"i == "<< i << (i<argc) << (i>argc);
        QVector<QVariant> targs = VALUE2Variant2(argv[i]);
        for (auto v: targs) args << v;
        qDebug()<<"i == "<< i << (i<argc) << (i>argc)<<targs;
    }

    return args;
}

/*
  统一的to_s方法
  
 */
VALUE x_Qt_meta_class_to_s(int argc, VALUE *argv, VALUE obj)
{
    qDebug()<<argc;
    QString klass_name = QString(rb_class2name(RBASIC_CLASS(obj)));
    void *ci = Qom::inst()->jdobjs[obj];
    
    QString stc; // stream container
    QDebug dm(&stc);
    dm << "tsed:" << klass_name << "Obj:"<< obj << "Hash:" << rb_hash(obj) << "C:"<< ci;
    // 怎么能解引用呢 *ci, 当前void*无法解。
    // dm << "hehe has newline???"; // 这种方式要手动换行。

    // 简单方法，
    if (klass_name == "Qt5:QString") dm << *(YaQString*)ci;
    else if (klass_name == "Qt5::QUrl") dm << *(YaQUrl*)ci;

    if (klass_name == "Qt5::QWidget") {
        QWidget *w = (QWidget*)ci;
    }
    
    // qDebug()<<stc;
    VALUE rv = rb_str_new2(stc.toLatin1().data());
    return rv;
}

#include "ctrlengine.h"
#include "frontengine.h"
#include "tests.cpp"

static CtrlEngine *gce = new CtrlEngine();

/*
  解决类中的enum的处理
  TODO:
  使用FrontEngine查找
 */
VALUE x_Qt_class_const_missing_jit(int argc, VALUE *argv, VALUE obj)
{
    qDebug()<<argc<<TYPE(obj)<<TYPE(argv[0]);
    qDebug()<<QString(rb_id2name(SYM2ID(argv[0])))<<rb_class2name(obj);
    QString klass_name = QString(rb_class2name(obj));
    klass_name = klass_name.split("::").at(1);
    QString const_name = QString(rb_id2name(SYM2ID(argv[0])));
    QString yconst_name = "y" + const_name;
    qDebug()<<klass_name<<const_name;

    int enum_val = gce->vm_enum(klass_name, const_name);
    qDebug()<<enum_val;

    return INT2NUM(enum_val);
    return Qnil;
}

/*
  解决类中的enum的处理
  TODO:
  使用staticMetaObject检测enum变量。
 */
VALUE x_Qt_meta_class_const_missing(int argc, VALUE *argv, VALUE obj)
{
    qDebug()<<argc<<TYPE(obj)<<TYPE(argv[0]);
    qDebug()<<QString(rb_id2name(SYM2ID(argv[0])))<<rb_class2name(obj);
    QString klass_name = QString(rb_class2name(obj));
    QString const_name = QString(rb_id2name(SYM2ID(argv[0])));
    QString yconst_name = "y" + const_name;
    // qDebug()<<klass_name<<const_name;

    if (klass_name == "Qt5::QHostAddress") {
        //  if (const_name == "AnyIPv4") return INT2NUM(QHostAddress::AnyIPv4);
    }
    
    VALUE *targv = NULL;
    int targc = 0;
    VALUE self = rb_class_new_instance(targc, targv, obj);
    GET_CI0();
    const QMetaObject *mo = ci->metaObject();
    // qDebug()<<"===="<<ci<<mo<<"----";

    // qDebug()<<"ec:"<<mo->enumeratorCount();
    for (int i = 0; i < mo->enumeratorCount(); i++) {
        QMetaEnum me = mo->enumerator(i);
        bool ok = false;
        // qDebug()<<"enum:"<<i<<""<<me.name();
        // qDebug()<<"enum2:"<<me.keyCount()<<me.keyToValue(const_name.toLatin1().data(), &ok)<<ok;
        int enum_val = me.keyToValue(yconst_name.toLatin1().data(), &ok);
        if (ok) {
            rb_gc_mark(self);
            return INT2NUM(enum_val);
        }

        continue; // below for debug/test purpose
        for (int j = 0; j < me.keyCount(); j++) {
            qDebug()<<"enum2:"<<j<<me.key(j);

            if (QString(me.key(i)) == yconst_name) {
                
            }
        }
    }

    // not found const
    VALUE exception = rb_eException;
    rb_raise(exception, "NameError: uninitialized yconstant %s::%s.\n",
             klass_name.toLatin1().data(), const_name.toLatin1().data());
    /*
    rb_fatal("NameError: uninitialized constant %s::%s.\n",
             klass_name.toLatin1().data(), const_name.toLatin1().data());
    */

    return Qnil;
}

static VALUE x_Qt_meta_class_dtor_jit(VALUE id)
{
    VALUE os = rb_const_get(rb_cModule, rb_intern("ObjectSpace"));
    VALUE self = rb_funcall(os, rb_intern("_id2ref"), 1, id);
    QString klass_name = QString(rb_class2name(RBASIC_CLASS(self)));
    klass_name = klass_name.split("::").at(1);
    qDebug()<<"dtor:"<<klass_name;

    void *ci = Qom::inst()->jdobjs[self];
    qDebug()<<"herhe:"<<ci;

    // TODO
    // 函数为void*时程序崩溃，这是llvm的bug还是使用有问题。
    QString code_src = QString("#include <QtCore>\n"
                               "void jit_main(int a, void *ci) {"
                               "qDebug()<<\"test int:\"<<a;"
                               "qDebug()<<\"in jit:\"<<ci;\n"
                               "delete (%1*)ci;}").arg(klass_name);
    QVector<llvm::GenericValue> gvargs;
    llvm::GenericValue ia;
    ia.IntVal = llvm::APInt(32, 567);
    gvargs.push_back(ia);
    gvargs.push_back(llvm::PTOGV(ci));
    qDebug()<<"view conv back:"<<llvm::GVTOP(gvargs.at(0));

    // delete (QString*)ci;
    llvm::GenericValue gvret = vm_execute(code_src, gvargs);

    return Qnil;
}

/*
  类实例的析构函数  
  TODO:
  使用通用方法之后，宏SAVE_XXX和GET_XXX就可以不需要了。
 */
static VALUE x_Qt_meta_class_dtor(VALUE id)
{
    VALUE os = rb_const_get(rb_cModule, rb_intern("ObjectSpace"));
    VALUE self = rb_funcall(os, rb_intern("_id2ref"), 1, id);

    GET_CI0();
    delete ci;

    return Qnil;
}

/////////////////////////////////////// jitttttt
/*
  类实例的析构函数  
  TODO:
  使用通用方法之后，宏SAVE_XXX和GET_XXX就可以不需要了。
 */
static VALUE x_Qt_class_dtor(VALUE id)
{
    VALUE os = rb_const_get(rb_cModule, rb_intern("ObjectSpace"));
    VALUE self = rb_funcall(os, rb_intern("_id2ref"), 1, id);

    // GET_CI0();
    // delete ci;

    // void* now, can not delete, need dtor and free
    void *qo = Qom::inst()->jdobjs[self];
    qDebug()<<qo<<rb_class2name(RBASIC_CLASS(self));

    return Qnil;
}

/*
// 这应该叫统一的方法，获取qt class名称
// get_qt_class(VALUE self)
// 判断是否是qt类，如果不是，很可能是在Ruby中继承Qt类
// 如果是继承类，取到需要实例化的Qt类名
TODO 还需要考虑继承层次大于2的情况
 */
static QString get_qt_class(VALUE self)
{
    QString klass_name = rb_class2name(RBASIC_CLASS(self));
    if (klass_name.startsWith("Qt5::Q")) return klass_name.split("::").at(1);

    // 如果是继承类，取到需要实例化的Qt类名
    VALUE pcls = RCLASS_SUPER(RBASIC_CLASS(self));
    klass_name = rb_class2name(pcls);
    return klass_name.split("::").at(1);
}

/*
  通过Qt类的初始化函数
  获取要实例化的类名，从staticMetaObject加载类信息，
  使用Qt的QMetaObject::newInstance创建新的实例对象。
  TODO:
  处理初始化时的参数。
 */
VALUE x_Qt_class_init_jit(int argc, VALUE *argv, VALUE self)
{
    qDebug()<<argc<<TYPE(self);
    qDebug()<<rb_class2name(RBASIC_CLASS(self));

    QString klass_name = get_qt_class(self);
    qDebug()<<"class name:"<<klass_name;


    // test_fe();
    // test_parse_class();
    // test_parse_ast();
    // test_piece_compiler();
    // test_tpl_piece_compiler();
    // exit(-1);
    // assert(1==2);
    
    QVector<QVariant> args;
    for (int i = 0; i < argc; i ++) {
        // if (i == 0) continue; // for ctor 0 is arg0
        if (i >= argc) break;
        qDebug()<<"i == "<< i << (i<argc) << (i>argc);
        args << VALUE2Variant(argv[i]);
        qDebug()<<"i == "<< i << (i<argc) << (i>argc)<<VALUE2Variant(argv[i]);
    }
    
    void *jo = gce->vm_new(klass_name, args);
    qDebug()<<jo<<self<<rb_hash(self);    

    Qom::inst()->jdobjs[self] = jo;

    VALUE free_proc = rb_proc_new(FUNVAL x_Qt_class_dtor, 0);
    rb_define_finalizer(self, free_proc);

    // test slot_ruby_space
    /*
    ID rbsym = rb_intern("slot_ruby_space");
    qDebug()<<"rb func:"<<(rbsym == Qnil);
    rb_funcall(rb_cObject, rbsym, 0);
    exit(0);
    */
    
    return self;
}

/*
  通过Qt类的初始化函数
  获取要实例化的类名，从staticMetaObject加载类信息，
  使用Qt的QMetaObject::newInstance创建新的实例对象。
  TODO:
  处理初始化时的参数。
 */
VALUE x_Qt_meta_class_init_jit(int argc, VALUE *argv, VALUE self)
{
    qDebug()<<argc<<TYPE(self);
    QString klass_name = QString(rb_class2name(RBASIC_CLASS(self)));
    klass_name = klass_name.split("::").at(1);
    QString yklass_name = QString("y%1").arg(klass_name);
    qDebug()<<"class name:"<<klass_name;

    // test_fe();
    // test_parse_class();
    // test_parse_ast();
    test_piece_compiler();
    exit(-1);
    
    auto code_templater = [] () -> QString * {
        return new QString();
    };

    QString code_src = QString("#include <stdio.h>\n"
                               "#include <QtCore>\n"
                               "%1 * jit_main() {\n"
                               "%1* ci = new %1(); qDebug()<<\"in jit:\"<<ci; return ci; }")
        .arg(klass_name);
    
    const char *code = "#include <stdio.h>\n"
        "#include <QtCore>\n"
        "\nint main() { printf (\"hello IR JIT from re.\"); QString abc; abc.append(\"123\"); qDebug()<<abc; return 56; }"
        "\nint yamain() { QString abc; abc.append(\"123\"); qDebug()<<abc; return main(); }";

    QVector<QVariant> args;
    void *jo = jit_vm_new(klass_name, args);
    qDebug()<<jo;
    Qom::inst()->jdobjs[self] = jo;

    if (0) {
        QVector<llvm::GenericValue> envp;
        // void *vret = vm_execute(QString(code), envp);

        llvm::GenericValue gvret = jit_vm_execute(code_src, envp);
        void *ci = llvm::GVTOP(gvret);
        Qom::inst()->jdobjs[self] = ci;
        qDebug()<<"newed ci:"<<ci;
    }

    VALUE free_proc = rb_proc_new(FUNVAL x_Qt_meta_class_dtor_jit, 0);
    // rb_define_finalizer(self, free_proc);

    if (0) {
        // QString *str = (QString*)llvm::GVTOP(gvret);
        // str->append("123456");
    
        // QString str2(*str);
        // qDebug()<<code_src<<*str<<str->length();
        // delete str; str = NULL;
    }

    return self;
}

VALUE x_Qt_meta_class_init(int argc, VALUE *argv, VALUE self)
{
    qDebug()<<argc<<TYPE(self);
    QString klass_name = QString(rb_class2name(RBASIC_CLASS(self)));
    klass_name = klass_name.split("::").at(1);
    QString yklass_name = QString("y%1").arg(klass_name);
    qDebug()<<"class name:"<<klass_name;
    
    if (!__rq_metas.contains(yklass_name)) {
        qDebug()<<"not supported class:"<<klass_name;
        return Qnil;
    }

    const QMetaObject *mo = __rq_metas.value(yklass_name);
    QObject * ci = mo->newInstance();
    SAVE_CI0(ci);

    VALUE free_proc = rb_proc_new(FUNVAL x_Qt_meta_class_dtor, 0);
    rb_define_finalizer(self, free_proc);

    return self;
    return Qnil;
}

// test method
/* 
// good test code for trace llvm ir call 
QString &test_ir_objref(YaQString *pthis, QString &str)
{
    qDebug()<<"pthis:"<<pthis;
    qDebug()<<"&str:"<<&str;
    qDebug()<<"str:"<<str;
    return str;
}
*/

/*
  stack structure:
  [0] => SYM function name
  [1] => arg0
  [2] => arg1
  [3] => arg2
  ...
 */
VALUE x_Qt_class_method_missing_jit(int argc, VALUE *argv, VALUE self)
{
    void *jo = Qom::inst()->jdobjs[self];
    void *ci = jo;
    qDebug()<<ci<<argc;
    assert(ci != 0);
    QString klass_name = get_qt_class(self);
    QString method_name = QString(rb_id2name(SYM2ID(argv[0])));
    qDebug()<<"calling:"<<klass_name<<method_name<<argc<<(argc > 1);
    assert(argc >= 1);

    QVector<QVariant> args;
    /*
    for (int i = 0; i < argc; i ++) {
        if (i == 0) continue;
        if (i >= argc) break;

        qDebug()<<"i == "<< i << (i<argc) << (i>argc);

        args << VALUE2Variant(argv[i]);
        qDebug()<<"i == "<< i << (i<argc) << (i>argc)<<VALUE2Variant(argv[i]);
    }
    */
    args = ARGV2Variant(argc, argv, 1);

    // fix try_convert(obj) → array or nil
    if (method_name == "to_ary") {
        return Qnil;
    }

    if (method_name == "to_s") {
        return Qnil;
    }

    // property assign
    if (method_name.endsWith('=')) {
        // TODO 需要更多的详细处理，通过类的property定义找到指定的赋值方法。
        QString set_method_name = QString("set%1%2")
            .arg(method_name.left(1).toUpper())
            .arg(method_name.mid(1, method_name.length()-2));
        gce->vm_call(ci, klass_name, set_method_name, args);
        return Qnil;
    }

    if (method_name.endsWith('?')) {
        qDebug()<<"need rubyfier adjust";
    }

    qDebug()<<ci<<klass_name<<method_name<<args;
    QVariant gv = gce->vm_call(ci, klass_name, method_name, args);
    qDebug()<<"vv:"<<gv;
    // TODO 使用真实的返回值
    if (method_name == "height" || method_name == "width"
        || method_name == "x" || method_name == "y") {
        return INT2NUM(gv.toInt());
    }
    
    return Qnil;
}

/*
  stack structure:
  self => CLASS
  [0] => SYM function name
  [1] => arg0
  [2] => arg1
  [3] => arg2
  ...
 */
VALUE x_Qt_class_singleton_method_missing_jit(int argc, VALUE *argv, VALUE self)
{
    qDebug()<<argc;
    QString klass_name = rb_class2name(self);
    klass_name = klass_name.split("::").at(1);
    QString method_name = QString(rb_id2name(SYM2ID(argv[0])));
    qDebug()<<"calling:"<<klass_name<<method_name<<argc<<(argc > 1);        
    

    QVector<QVariant> args;
    args = ARGV2Variant(argc, argv, 1);

    // fix try_convert(obj) → array or nil
    if (method_name == "to_ary") {
        return Qnil;
    }

    if (method_name == "to_s") {
        return Qnil;
    }

    QVariant gv = gce->vm_static_call(klass_name, method_name, args);
    qDebug()<<"vv:"<<gv;
    
    return Qnil;
}

/*
  stack structure:
  [0] => SYM function name
  [1] => arg0
  [2] => arg1
  [3] => arg2
  ...
 */
VALUE x_Qt_meta_class_method_missing_jit(int argc, VALUE *argv, VALUE self)
{
    void *jo = Qom::inst()->jdobjs[self];
    void *ci = jo;
    qDebug()<<ci;
    assert(ci != 0);
    QString klass_name = QString(rb_class2name(RBASIC_CLASS(self)));
    klass_name = klass_name.split("::").at(1);
    QString method_name = QString(rb_id2name(SYM2ID(argv[0])));
    qDebug()<<"calling:"<<klass_name<<method_name<<argc<<(argc > 1);
    assert(argc >= 1);

    QVector<QVariant> args;
    for (int i = 0; i < argc; i ++) {
        if (i == 0) continue;
        if (i >= argc) break;

        qDebug()<<"i == "<< i << (i<argc) << (i>argc);

        args << VALUE2Variant(argv[i]);
        qDebug()<<"i == "<< i << (i<argc) << (i>argc)<<VALUE2Variant(argv[i]);
    }

    // fix try_convert(obj) → array or nil
    if (method_name == "to_ary") {
        return Qnil;
    }

    if (method_name == "to_s") {
        return Qnil;
    }

    /*
    YaQString *ts = (YaQString*)ci;
    ts->append("1234abc");
    qDebug()<<(*ts)<<ts->toUpper()<<ts->startsWith(QChar('r'))
            <<ts->lastIndexOf("876");
    */

    QVariant gv = jit_vm_call(ci, klass_name, method_name, args);
    qDebug()<<"gv:"<<gv;

    return Qnil;
}

/*
  stack structure:
  [0] => SYM function name
  [1] => arg0
  [2] => arg1
  [3] => arg2
  ...
 */
VALUE x_Qt_meta_class_method_missing(int argc, VALUE *argv, VALUE self)
{
    GET_CI0();
    qDebug()<<ci;
    assert(ci != 0);
    const QMetaObject *mo = ci->metaObject();
    QString klass_name = "QString";
    QString method_name = QString(rb_id2name(SYM2ID(argv[0])));
    qDebug()<<"calling:"<<klass_name<<method_name<<argc<<(argc > 1);
    assert(argc >= 1);

    QVector<QVariant> args;
    for (int i = 0; i < argc; i ++) {
        if (i == 0) continue;
        if (i >= argc) break;

        qDebug()<<"i == "<< i << (i<argc) << (i>argc);

        args << VALUE2Variant(argv[i]);
        qDebug()<<"i == "<< i << (i<argc) << (i>argc)<<VALUE2Variant(argv[i]);
    }

    // fix try_convert(obj) → array or nil
    if (method_name == "to_ary") {
        return Qnil;
    }

    if (method_name == "to_s") {
        return Qnil;
    }
    
    // auto meta invoke
    QString msig;
    for (QVariant &item : args) {
        msig.append(item.typeName()).append(',');
    }

    msig[msig.length()-1] = msig[msig.length()-1] == ',' ? QChar(' ') : msig[msig.length()-1];
    msig = msig.trimmed();
    msig = QString("%1(%2)").arg(method_name).arg(msig);

    int midx = mo->indexOfMethod(msig.toLatin1().data());
    if (midx == -1) {
        for (int i = 0; i < mo->methodCount(); i ++) {
            QMetaMethod mm = mo->method(i);
            qDebug()<<"mmsig:"<<mm.returnType()<<mm.typeName()<<mm.methodSignature();
            if (QString(mm.methodSignature()).replace("const ", "") == msig) {
                midx = i;
                break;
            }
        }
    } 

    if (midx == -1) {
        qDebug()<<"method not found:"<<msig;
    } else {        
        QMetaMethod mm = mo->method(midx);
        // qDebug()<<"mmsig:"<<mm.methodSignature();
        ReturnStorage rs;
        InvokeStorage is;
        QGenericReturnArgument retval;
        QGenericArgument val0, val1, val2, val3, val4, val5, val6, val7, val8, val9;
        int rargc = argc - 1;
        qDebug()<<"retun type:"<<mm.returnType()<<mm.typeName()<<mm.parameterCount()<<rargc;

        if (argc - 1 > mm.parameterCount()) {
            qDebug()<<"maybe you passed too much parameters:"
                    <<QString("need %1, given %2.").arg(mm.parameterCount()).arg(rargc);
        }

        retval = makeRetArg(mm.returnType(), rs);

        bool bret = false;
        // QString tmpv0;
        switch (rargc) {
        case 0:
            // ci->append("123");
            bret = mm.invoke(ci, retval);//Q_RETURN_ARG(int, retval)); 
            break;
        case 1:
            val0 = Variant2Arg(mm.parameterType(0), args[0], 0, is);
            bret = mm.invoke(ci, retval, val0);
            
            // tmpv0 = args[0].toString(); // 如果使用在Q_ARG中直接使用这个，这个临时对象地址会消失
            // val0 = Q_ARG(QString, tmpv0);
            // bret = QMetaObject::invokeMethod(ci, "append", val0);
            // bret = mm.invoke(ci, retval, val0);
            break;
        case 2:
            val0 = Variant2Arg(mm.parameterType(0), args[0], 0, is);
            val1 = Variant2Arg(mm.parameterType(1), args[1], 1, is);
            bret = mm.invoke(ci, retval, val0, val1);
            break;
        case 3:
            val0 = Variant2Arg(mm.parameterType(0), args[0], 0, is);
            val1 = Variant2Arg(mm.parameterType(1), args[1], 1, is);
            val2 = Variant2Arg(mm.parameterType(2), args[2], 2, is);
            bret = mm.invoke(ci, retval, val0, val1, val2);
            break;
        case 4:
            qDebug()<<"invokkkk:"<<rargc<<4;
            val0 = Variant2Arg(mm.parameterType(0), args[0], 0, is);
            val1 = Variant2Arg(mm.parameterType(1), args[1], 1, is);
            val2 = Variant2Arg(mm.parameterType(2), args[2], 2, is);
            val3 = Variant2Arg(mm.parameterType(3), args[3], 3, is);
            bret = mm.invoke(ci, retval, val0, val1, val2, val3);
            break;
        case 5:
            val0 = Variant2Arg(mm.parameterType(0), args[0], 0, is);
            val1 = Variant2Arg(mm.parameterType(1), args[1], 1, is);
            val2 = Variant2Arg(mm.parameterType(2), args[2], 2, is);
            val3 = Variant2Arg(mm.parameterType(3), args[3], 3, is);
            val4 = Variant2Arg(mm.parameterType(4), args[4], 4, is);
            bret = mm.invoke(ci, retval, val0, val1, val2, val3, val4);
            break;
        default:
            qDebug()<<"not impled"<<argc;
            break;
        };
        qDebug()<<bret<<","<<retval.name()<<retval.data();

        VALUE my_retval = retArg2Value(mm.returnType(), retval);
        return my_retval;
    }

    return Qnil;
}


// static VALUE x_QString_method_missing_test(int argc, VALUE *argv, VALUE self);
/*
static VALUE x_QString_method_missing(int argc, VALUE *argv, VALUE self)
{
    GET_CI3(QString);
    qDebug()<<ci;
    
    QString method_name = QString(rb_id2name(SYM2ID(argv[0])));
    QString klass_name = "QString"; // TODO, dynamic way
    qDebug()<<"calling:"<<klass_name<<method_name<<argc<<(argc > 1);

    QVector<QVariant> args;
    for (int i = 0; i < argc; i ++) {
        if (i == 0) continue;
        if (i >= argc) break;

        qDebug()<<"i == "<< i << (i<argc) << (i>argc);

        args << VALUE2Variant(argv[i]);
        qDebug()<<"i == "<< i << (i<argc) << (i>argc)<<VALUE2Variant(argv[i]);
    }

    // fix try_convert(obj) → array or nil
    if (method_name == "to_ary") {
        return Qnil;
    }

    if (method_name == "to_s") {
        return Qnil;
    }

    // auto meta invoke
    QString msig;
    for (QVariant &item : args) {
        msig.append(item.typeName()).append(',');
    }

    msig[msig.length()-1] = msig[msig.length()-1] == ',' ? QChar(' ') : msig[msig.length()-1];
    msig = msig.trimmed();
    msig = QString("%1(%2)").arg(method_name).arg(msig);

    const QMetaObject *mo = ci->metaObject();
    int midx = mo->indexOfMethod(msig.toLatin1().data());
    if (midx == -1) {
        for (int i = 0; i < mo->methodCount(); i ++) {
            QMetaMethod mm = mo->method(i);
            qDebug()<<mm.methodSignature();
            if (QString(mm.methodSignature()).replace("const ", "") == msig) {
                midx = i;
                break;
            }
        }
    } 

    if (midx == -1) {
        qDebug()<<"method not found:"<<msig;
    } else {
        QMetaMethod mm = mo->method(midx);
        ReturnStorage rs;
        InvokeStorage is;
        QGenericReturnArgument retval;
        QGenericArgument val0, val1, val2, val3, val4, val5, val6, val7, val8, val9;
        // QVariant retval;
        int iretval = 1234567;
        QString sretval;
        QString sval[10] = {0};
        int ival[10] = {0};
        qDebug()<<"retun type:"<<mm.returnType()<<mm.typeName()<<mm.parameterCount();

        if (argc - 1 > mm.parameterCount()) {
            qDebug()<<"maybe you passed too much parameters:"
                    <<QString("need %1, given %2.").arg(mm.parameterCount()).arg(argc - 1);
        }

        retval = makeRetArg(mm.returnType(), rs);

        bool bret = false;
        switch (argc - 1) {
        case 0:
            // ci->append("123");
            bret = mm.invoke(ci, retval);//Q_RETURN_ARG(int, retval)); 
            break;
        case 1:
            val0 = Variant2Arg(mm.parameterType(0), args[0], 0, is);
            bret = mm.invoke(ci, retval, val0);
            break;
        case 2:
            val0 = Variant2Arg(mm.parameterType(0), args[0], 0, is);
            val1 = Variant2Arg(mm.parameterType(1), args[1], 1, is);
            bret = mm.invoke(ci, retval, val0, val1);
            break;
        case 3:
            val0 = Variant2Arg(mm.parameterType(0), args[0], 0, is);
            val1 = Variant2Arg(mm.parameterType(1), args[1], 1, is);
            val2 = Variant2Arg(mm.parameterType(2), args[2], 2, is);
            bret = mm.invoke(ci, retval, val0, val1, val2);
            break;
        case 4:
            val0 = Variant2Arg(mm.parameterType(0), args[0], 0, is);
            val1 = Variant2Arg(mm.parameterType(1), args[1], 1, is);
            val2 = Variant2Arg(mm.parameterType(2), args[2], 2, is);
            val3 = Variant2Arg(mm.parameterType(3), args[3], 3, is);
            bret = mm.invoke(ci, retval, val0, val1, val2, val3);
            break;
        case 5:
            val0 = Variant2Arg(mm.parameterType(0), args[0], 0, is);
            val1 = Variant2Arg(mm.parameterType(1), args[1], 1, is);
            val2 = Variant2Arg(mm.parameterType(2), args[2], 2, is);
            val3 = Variant2Arg(mm.parameterType(3), args[3], 3, is);
            val4 = Variant2Arg(mm.parameterType(4), args[4], 4, is);
            bret = mm.invoke(ci, retval, val0, val1, val2, val3, val4);
            break;
        default:
            qDebug()<<"not impled"<<argc;
            break;
        };
        qDebug()<<bret<<","<<retval.name()<<retval.data()<<iretval
                <<((QString*)ci)->toLatin1()<<((QString*)ci)->length();

        VALUE my_retval = retArg2Value(mm.returnType(), retval);
        return my_retval;
    }

    // x_QString_method_missing_test(argc, argv, self);    
    return Qnil; // very importent, no return cause crash
}

static VALUE x_QString_method_missing_test(int argc, VALUE *argv, VALUE self)
{
    GET_CI3(QString);
    
    QString method_name = QString(rb_id2name(SYM2ID(argv[0])));
    QString klass_name = "QString"; // TODO, dynamic way
    qDebug()<<"calling:"<<klass_name<<method_name;

    QVector<QVariant> args;

    for (int i = 1; i < argc; i++) {
        args << VALUE2Variant(argv[i]);
    }

    const QMetaObject * mo = NULL;
    // YaQString ystr;
    // mo = ystr.metaObject();
    // qDebug()<<"ycall:"<<ystr.append("123");
    
    QString msig = "append(";
    for (int i = 1; i < argc; i++) {
        msig += QString(args[i-1].typeName());
        if (i < argc - 1) msig += ",";
    }
    msig += ")";
    // no space allowed in msig string. also no ref char &, but have const keyword.but dont need return value
    msig = "append(const char*)";
    qDebug()<<"method sig:"<<msig << mo->indexOfMethod(msig.toLatin1().data());

    for (int i = 0; i < mo->methodCount(); i++) {
        QMetaMethod mi = mo->method(i);
        qDebug()<< i<< mo->method(i).methodSignature()
                <<mi.returnType()
                <<QMetaObject::checkConnectArgs(msig.toLatin1().data(), mi.methodSignature());
    }


    if (method_name == "append") {
        switch (argc - 1) {
            // case 1: ci->append(VOValue(args[0])); break;
            // callrr(ci, method_name.toLatin1().data(), VOValue(args[0]));
            // case 2: ci->append(VOValue(args[0]), VOValue(args[1])); break;
        default:
            break;
        }
    }

    
    if (method_name == "length") {
        int len = std::bind(&QString::length, ci)();
        qDebug()<<method_name<<"=>"<<len;
        return rb_int2inum(len);
    }


    // void **pfun = Qom::inst()->getfp(klass_name, method_name);
    // assert(pfun != NULL);

    // qDebug()<<pfun;


    QString pieceCode = "";

    pieceCode += klass_name+"::"+"append(";
    for (int i = 1; i < argc; i++) {
        switch (TYPE(argv[i])) {
        case T_FIXNUM:
            pieceCode += "int ";
            break;
        case T_STRING:
            pieceCode += "const char * " + QString(RSTRING_PTR(argv[i])) + ",";
            break;
        }
    }
    if (pieceCode.at(pieceCode.length() - 1) == ',') 
        pieceCode = pieceCode.left(pieceCode.length() - 1);
    pieceCode += ");";


    qDebug()<<"auto gen code:" << pieceCode;

    // std::function<void()> ftor = std::bind(*pfun, ci);
    // ftor();
    // ci->*(*pfun)();
    // int (QString::*p)() const = (int (QString::*)())(*pfun);

    for (int i = 0; i < argc; i++) {
        qDebug()<<__FUNCTION__<<TYPE(argv[i]);
        switch (TYPE(argv[i])) {
        case T_STRING:
            qDebug()<<i<<"is string:"<<RSTRING_PTR(argv[i]);
            break;
        case T_SYMBOL:
            qDebug()<<i<<"is symbol:"<<rb_id2name(SYM2ID(argv[i]));
            break;
        case T_FIXNUM:
            qDebug()<<i<<"is num:"<<FIX2INT(argv[i]);
            break;
        default:
            qDebug()<<__FUNCTION__<<i<<",unknown arg type:"<<TYPE(argv[i]);
            break;
        }
    }

    // typeid(ci);

    qDebug()<<__FUNCTION__<<TYPE(self)<<TYPE(argv[0])<<argc;
    // qDebug()<<"'"<<rb_id2name(argv[0])<<"'";

    // Qom::inst()->testParser();
    Qom::inst()->testIR();

    if (0) {
    QString methodName = "startsWith";
    QString str = "rubyoo afwefafe";

    QLibrary core("/usr/lib/libQt5Core.so");
    QFunctionPointer fsym = core.resolve(methodName.toLatin1().data());
    }

    // bool t = fsym()(&str, "ruby");
    // qDebug()<<fsym<<t;


    if (0) {
         str.methodName(args);
         bind(fsym, &str);
         QVariant ret = std::bind(&QString::methodName, &str, _1, _2, _3 ...)();
         return ret;
      }
    



    return Qnil;
}
*/

// like class's static method
VALUE x_Qt_meta_class_singleton_method_missing(int argc, VALUE *argv, VALUE obj)
{
    qDebug()<<argc<<TYPE(argv[0])<<TYPE(obj);

    // const QMetaObject *mo = &yQObject::staticMetaObject;
    
    // qDebug()<<"qApp:"<<qApp;
    // for (int i = 0; i < mo->methodCount(); i++) {
    //     QMetaMethod mi = mo->method(i);
    //     qDebug()<< i<< mo->method(i).methodSignature()
    //             <<mi.returnType();
    // }

    // QString klass_name = QString(rb_class2name(obj));
    // QString const_name = QString(rb_id2name(SYM2ID(argv[0])));
    // qDebug()<<klass_name<<const_name;
    // qDebug()<<SLOT(on_emu_signal(int))<<SLOT(on_emu_signal())
    //         <<SIGNAL(emu_signal(int))<<SIGNAL(emu_signal());
    // // 1on_emu_signal(int) 1on_emu_signal()

    // yQObject *to = new yQObject;
    // if (klass_name == "Qt5::QObject") {
    //     if (const_name == "connect") {
    //         // standard connect
    //         QString slot_sig = QString("1%1").arg(RSTRING_PTR(argv[4]));
    //         QString signal_sig = QString("2%1").arg(RSTRING_PTR(argv[2]));
    //         QObject *slot_o = Qom::inst()->objs[rb_hash(argv[3])];
    //         QObject *signal_o = Qom::inst()->objs[rb_hash(argv[1])];
    //         qDebug()<<"slotooo:"<<slot_sig<<signal_sig<<slot_o<<signal_o;

    //         QObject::connect(signal_o, signal_sig.toLatin1().data(),
    //                          slot_o, slot_sig.toLatin1().data());

    //         // 
    //         // 
    //         // emit(signal_o->emu_signal(3));
    //         // another
    //     } else { // emit
    //         qDebug()<<"calling emit";
    //         const QMetaObject * mo = &yQObject::staticMetaObject;
    //         QObject *io = mo->newInstance();
    //         QObject *io2 = (decltype(io))io;
    //         qDebug()<<io<<io2;
    //     }
    // }

    return Qnil;
}


static VALUE x_QApplicatiton_init(VALUE self)
{
    int argc = 1;
    char *argv[] = {(char*)"handby"};
    QApplication *app = new QApplication(argc, argv);

    SAVE_CI2(QApplication, app);

    return self;
}

static VALUE x_QApplication_exec(VALUE self)
{
    GET_CI2(QApplication);

    int n = ci->exec();

    return n;
}

// for qApp 等全局变量
VALUE x_Qt_global_variable_get(ID id, VALUE *data, struct global_entry *entry)
{
    // qDebug()<<id<<data<<entry;
    // qDebug()<<rb_id2name(id)<<qApp;
    QString vname = rb_id2name(id);

    if (vname == "$qApp") {
        void *v = qApp;
        if (v == NULL) return Qnil;
        // TODO 提高查找效率。
        for (auto it = Qom::inst()->jdobjs.begin(); it != Qom::inst()->jdobjs.end(); it++) {
            if (it.value() == v) {
                return it.key();
            }
        }
        return Qnil;
    }
    qDebug()<<"undef variable:"<<vname;
    return INT2NUM(123456);
    return Qnil;
}

void x_Qt_global_variable_set(VALUE value, ID id, VALUE *data, struct global_entry *entry)
{
    qDebug()<<TYPE(value)<<id<<data<<entry;
    return;
}

static VALUE x_Qt_Constant_missing(int argc, VALUE* argv, VALUE self)
{
    qDebug()<<"hehhe constant missing."<<argc<<TYPE(self);

    QString vspace = "Qt";
    QString vname;
    
    // assert(argc == 1);
    // assert(TYPE(self) == T_SYMBOL)
    for (int i = 0; i < argc; i++) {
        qDebug()<<__FUNCTION__<<TYPE(argv[i]);
        switch (TYPE(argv[i])) {
        case T_STRING:
            qDebug()<<i<<"is string:"<<RSTRING_PTR(argv[i]);
            vname = RSTRING_PTR(argv[i]);
            break;
        case T_SYMBOL:
            qDebug()<<i<<"is symbol:"<<rb_id2name(SYM2ID(argv[i]));
            vname = rb_id2name(SYM2ID(argv[i]));
            break;
        case T_FIXNUM:
            qDebug()<<i<<"is num:"<<FIX2INT(argv[i]);
            break;
        default:
            qDebug()<<__FUNCTION__<<i<<",unknown arg type:"<<TYPE(argv[i]);
            break;
        }
    }

    // vm exec
    int val = gce->vm_enum(vspace, vname);
    qDebug()<<"enum..."<<val;

    return INT2NUM(val);
    return Qnil;
}

// from qtobjectmanager.h
void ConnectProxy::proxycall()
{
    qDebug()<<"aaaaaaaa"<<sender()<<senderSignalIndex();
    qDebug()<<this->osender<<this->osignal<<this->oreceiver<<this->oslot;
    qDebug()<<this->argc<<this->argv<<this->self;
    // ID slot = rb_intern("slot_ruby_space");
    if (this->argc == 4) {
        ID slot = rb_intern(this->oslot.toLatin1().data());
        rb_funcall(rb_cObject, slot, 0);
    }
    else if (this->argc == 5) {
        qDebug()<<"called....";
        ID slot = rb_intern(this->oslot.toLatin1().data());
        rb_funcall(this->oreceiver, slot, 0);
    }
    else {
        qDebug()<<"unimpled.....";
    }
}

void ConnectProxy::proxycall(int arg0)
{
    qDebug()<<"aaaaaaaa"<<sender();
    ID slot = rb_intern("slot_ruby_space");
    rb_funcall(rb_cObject, slot, 0);
}

void ConnectProxy::proxycall(char arg0)
{
    qDebug()<<"aaaaaaaa"<<sender();
    ID slot = rb_intern("slot_ruby_space");
    rb_funcall(rb_cObject, slot, 0);
}

void ConnectProxy::proxycall(void * arg0)
{
    qDebug()<<"aaaaaaaa"<<sender();
    ID slot = rb_intern("slot_ruby_space");
    rb_funcall(rb_cObject, slot, 0);
}

static ConnectProxy gcp;
static VALUE x_Qt_connectrb(int argc, VALUE* argv, VALUE self)
{
    if (argc == 4) {
        QVariant vqobj = VALUE2Variant(argv[1]);
        QVariant vsignal = VALUE2Variant(argv[2]);
        QString signal = vsignal.toString();
        QString rsignal = QString("2%1").arg(signal); // real signal
        auto qobj = (QObject*)(vqobj.value<void*>());
        auto spec_qobj = (QTimer*)(vqobj.value<void*>()); // 物化对象
    
        // QObject::connect(qobj, SIGNAL(timeout()), qobj, SLOT(stop())); // ok
        // QObject::connect(qobj, SIGNAL(timeout()), [](){}); // error
        // auto conn = QObject::connect(qobj, &QTimer::timeout, // vsignal.toString().toLatin1().data(),
        //                              [=]() {
        //                                  qDebug()<<argc<<argv<<self;
        //                                  ID slot = rb_intern("slot_ruby_space");
        //                                  rb_funcall(rb_cObject, slot, 0);
        //                              });

        // TODO 什么时候free掉这个对象？？？
        ConnectProxy *connpxy = new ConnectProxy();
        connpxy->argc = argc;
        connpxy->argv = (RB_VALUE*)argv;
        connpxy->self = self;

        connpxy->osender = qobj;
        connpxy->osignal = rsignal;
        connpxy->oreceiver = (RB_VALUE)NULL;
        connpxy->oslot = rb_id2name(SYM2ID(argv[3]));

        QString slot = QString("1proxycall%1")
            .arg(signal.right(signal.length() - signal.indexOf('(')));
        qDebug()<<"connecting singal/slot........"<<signal.indexOf('(')<<slot;
        auto conn = QObject::connect(qobj, rsignal.toLatin1().data(), connpxy, slot.toLatin1().data()); // ok
        auto cid = connpxy->addConnection(conn);
    }
    else if (argc == 5) {
        QVariant vqobj = VALUE2Variant(argv[1]);
        QVariant vsignal = VALUE2Variant(argv[2]);
        QString signal = vsignal.toString();
        QString rsignal = QString("2%1").arg(signal); // real signal
        auto qobj = (QObject*)(vqobj.value<void*>());
        auto spec_qobj = (QTimer*)(vqobj.value<void*>()); // 物化对象
    
        // QObject::connect(qobj, SIGNAL(timeout()), qobj, SLOT(stop())); // ok
        // QObject::connect(qobj, SIGNAL(timeout()), [](){}); // error
        // auto conn = QObject::connect(qobj, &QTimer::timeout, // vsignal.toString().toLatin1().data(),
        //                              [=]() {
        //                                  qDebug()<<argc<<argv<<self;
        //                                  ID slot = rb_intern("slot_ruby_space");
        //                                  rb_funcall(rb_cObject, slot, 0);
        //                              });

        // TODO 什么时候free掉这个对象？？？
        ConnectProxy *connpxy = new ConnectProxy();
        connpxy->argc = argc;
        connpxy->argv = (RB_VALUE*)argv;
        connpxy->self = self;

        connpxy->osender = qobj;
        connpxy->osignal = rsignal;
        connpxy->oreceiver = argv[3];
        connpxy->oslot = rb_id2name(SYM2ID(argv[4]));

        QString slot = QString("1proxycall%1")
            .arg(signal.right(signal.length() - signal.indexOf('(')));
        qDebug()<<"connecting singal/slot........"<<signal.indexOf('(')<<slot;
        auto conn = QObject::connect(qobj, rsignal.toLatin1().data(), connpxy, slot.toLatin1().data()); // ok
        auto cid = connpxy->addConnection(conn);
    } else {
        qDebug()<<"unimpled...!!!";
    }
    
    return Qnil;
}

static VALUE x_Qt_Method_missing(int argc, VALUE* argv, VALUE self)
{
    qDebug()<<"hehhe method missing."<< argc << argv << self;
    qDebug()<<TYPE(self); // == 3(T_MODULE)
    // RSTRING_PTR(argv[0]);
    for (int i = 0; i < argc; i ++) {
        qDebug()<<i<<TYPE(argv[i])<<VALUE2Variant(argv[i]);
    }
    // argv[0] is method symbol
    // argv[1] is method arg0
    // argv[2] is method arg1
    // ...
    QString method_name = rb_id2name(SYM2ID(argv[0]));
    qDebug()<<method_name;
    if (method_name == "connectrb") {
        qDebug()<<"got ittttttt."<<method_name;
        return x_Qt_connectrb(argc, argv, self);
    }

    qDebug()<<"unimpled!!!";
    
    return Qnil;
}


extern "C" {
    // VALUE cQString;
    // VALUE cQApplication;
    VALUE cModuleQt;
    
    void Init_handby()
    {
        qInstallMessageHandler(myMessageOutput);

        ///// test code
        // test_one();
        // exit(0);

        init_class_metas();

        cModuleQt = rb_define_module("Qt5");
        // 对所有的Qt5::someconst常量的调用注册
        rb_define_module_function(cModuleQt, "const_missing", FUNVAL x_Qt_Constant_missing, -1);
        // 对所有的Qt5::somefunc()函数的调用注册
        rb_define_module_function(cModuleQt, "method_missing", FUNVAL x_Qt_Method_missing, -1);

        // 所有Qt类的initialize, method_missing, const_missing函数注册
        register_qtruby_classes(cModuleQt);

        // register_QCoreApplication_methods(cModuleQt);
        // register_qtcore_methods(cModuleQt);
        // register_qtnetwork_methods(cModuleQt);

        // cQByteArray = rb_define_class("QByteArray", rb_cObject);
        // rb_define_method(cQByteArray, "initialize", FUNVAL x_QByteArray_init, 0);

        // cQThread = rb_define_class("QThread", rb_cObject);
        // rb_define_method(cQThread, "initializer", FUNVAL x_QThread_init, 0);

        // cQWidget = rb_define_class("QWidget", rb_cObject);
        // rb_define_method(cQWidget, "initialize", FUNVAL x_QWidget_init, 0);


        // 
        /*
        cQString = rb_define_class("QString", rb_cObject);
        rb_define_method(cQString, "initialize", (VALUE (*) (...)) x_QString_init, 0);
        // rb_define_method(cQString, "append", (VALUE (*) (...)) x_QString_append, 1);
        rb_define_method(cQString, "to_s", (VALUE (*) (...)) x_QString_to_s, 0);
        rb_define_method(cQString, "method_missing", FUNVAL x_QString_method_missing, -1);
        */

        // 
        // cQApplication = rb_define_class("QApplication", rb_cObject);
        // rb_define_method(cQApplication, "initialize", FUNVAL x_QApplication_init, 0);
        // rb_define_method(cQApplication, "exec", FUNVAL x_QApplication_exec, 0);

    }

};// end extern "C"



