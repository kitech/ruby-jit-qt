
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
#include "qom.h"

#include "utils.h"
#include "entry.h"

#include "clvm_letacy.h"
#include "clvm.h"


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

static QVariant VALUE2Variant(VALUE v)
{
    QVariant rv;

    switch (TYPE(v)) {
    case T_NONE:  rv = QVariant(); break;
    case T_FIXNUM: rv = (int)FIX2INT(v); break;
    case T_STRING:  rv = RSTRING_PTR(v); break;
    case T_FLOAT:  rv = RFLOAT_VALUE(v); break;
    case T_NIL:   rv = 0; break;
    case T_TRUE:  rv = true; break;
    case T_FALSE: rv = false; break;
    case T_CLASS:
        
    default:
        qDebug()<<"unknown VALUE type:"<<TYPE(v);
        break;
    }

    return rv;
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

    return Qnil;
}

static Clvm * gclvm = NULL;
static VALUE x_Qt_meta_class_dtor_jit(VALUE id)
{
    VALUE os = rb_const_get(rb_cModule, rb_intern("ObjectSpace"));
    VALUE self = rb_funcall(os, rb_intern("_id2ref"), 1, id);
    QString klass_name = QString(rb_class2name(RBASIC_CLASS(self)));
    klass_name = klass_name.split("::").at(1);
    qDebug()<<"dtor:"<<klass_name;

    void *ci = Qom::inst()->jdobjs[rb_hash(self)];
    qDebug()<<"herhe:"<<ci;

    // TODO
    // 函数为void*时程序崩溃，这是llvm的bug还是使用有问题。
    QString code_src = QString("#include <QtCore>\n"
                               "void jit_main(int a, void *ci) {"
                               "qDebug()<<\"test int:\"<<a;"
                               "qDebug()<<\"in jit:\"<<ci;\n"
                               "delete (%1*)ci;}").arg(klass_name);

    if (0) {
        QVector<llvm::GenericValue> gvargs;
        llvm::GenericValue ia;
        ia.IntVal = llvm::APInt(32, 567);
        gvargs.push_back(ia);
        gvargs.push_back(llvm::PTOGV(ci));
        qDebug()<<"view conv back:"<<llvm::GVTOP(gvargs.at(0));

        // delete (QString*)ci;
        llvm::GenericValue gvret = vm_execute(code_src, gvargs);
    }

    if (1) {
        // Clvm *vm = new Clvm();
        Clvm *vm = gclvm;
        std::vector<llvm::GenericValue> gvargs2;
        llvm::GenericValue ia2;
        ia2.IntVal = llvm::APInt(32, 789);
        gvargs2.push_back(ia2);
        gvargs2.push_back(llvm::PTOGV(ci));
        // vm->execute(code_src, gvargs2, "jit_main");
    }

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
    QVector<llvm::GenericValue> envp;
    std::vector<llvm::GenericValue> gvargs;
    // void *vret = vm_execute(QString(code), envp);

    if (1) {
        Clvm *vm = new Clvm();
        llvm::GenericValue gvret = vm->execute(code_src, gvargs, "jit_main");
        void *ci = llvm::GVTOP(gvret);
        Qom::inst()->jdobjs[rb_hash(self)] = ci;
        qDebug()<<"newed ci:"<<ci;
        // delete vm;
        gclvm = vm;
    }

    if (0) {
        llvm::GenericValue gvret = vm_execute(code_src, envp);
        void *ci = llvm::GVTOP(gvret);
        Qom::inst()->jdobjs[rb_hash(self)] = ci;
        qDebug()<<"newed ci:"<<ci;
    }

    VALUE free_proc = rb_proc_new(FUNVAL x_Qt_meta_class_dtor_jit, 0);
    rb_define_finalizer(self, free_proc);

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
    

    return self;
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
    void *ci = Qom::inst()->jdobjs[rb_hash(self)];
    qDebug()<<ci;
    assert(ci != 0);
    QString klass_name = QString(rb_class2name(RBASIC_CLASS((self)))); // "QString";
    klass_name = klass_name.split("::").at(1);
    QString method_name = QString(rb_id2name(SYM2ID(argv[0])));
    qDebug()<<"calling:"<<klass_name<<method_name<<argc<<(argc > 1);
    assert(argc >= 1);

    QString code_src = u8R"code(
#include <QtCore>
void jit_main(int a, %1 *ci) {
   // %1 *jci = (%1*)ci;
   ci->%2();
}
)code";
    code_src = code_src.arg(klass_name).arg(method_name);
    qDebug()<<code_src;

    
    std::vector<llvm::GenericValue> args;
    llvm::GenericValue ia;
    ia.IntVal = llvm::APInt(32, 567);
    args.push_back(ia);
    args.push_back(llvm::PTOGV(ci));
    Clvm *vm = gclvm;
    llvm::GenericValue rgv = vm->execute(code_src, args, "jit_main");
    qDebug()<<rgv.IntVal.getZExtValue();

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


// like class's static method
VALUE x_Qt_meta_class_singleton_method_missing(int argc, VALUE *argv, VALUE obj)
{
    qDebug()<<argc<<TYPE(argv[0])<<TYPE(obj);


    return Qnil;
}



static VALUE x_Qt_Constant_missing(int argc, VALUE* argv, VALUE self)
{
    qDebug()<<"hehhe constant missing."<<argc<<TYPE(self);

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

    return 0;
}

static VALUE x_Qt_Method_missing(int argc, VALUE* argv, VALUE self)
{
    qDebug()<<"hehhe method missing."<< RSTRING_PTR(argv[0]);
    return 0;
}


extern "C" {
    VALUE cQString;
    VALUE cModuleQt;

    void Init_handby()
    {
        qInstallMessageHandler(myMessageOutput);

        cModuleQt = rb_define_module("Qt5");
        rb_define_module_function(cModuleQt, "const_missing", FUNVAL x_Qt_Constant_missing, -1);
        // rb_define_module_function(cModuleQt, "method_missing", FUNVAL x_Qt_Method_missing, -1);

        // 
        cQString = rb_define_class_under(cModuleQt, "QString", rb_cObject);
        rb_define_method(cQString, "initialize", (VALUE (*) (...)) x_Qt_meta_class_init_jit, -1);
        rb_define_method(cQString, "method_missing", FUNVAL x_Qt_meta_class_method_missing_jit, -1);

    }

};// end extern "C"



