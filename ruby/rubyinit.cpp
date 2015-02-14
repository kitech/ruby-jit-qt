#include "debugoutput.h"

#include "ctrlengine.h"
#include "marshallruby.h"
#include "callargument.h"
#include "connectruby.h"

#include "utils.h"
#include "ruby_cxx.h"
#include "rubyinit.h"

static RubyInit *rbinit = NULL;
extern "C" void Init_handby()
{
    RubyInit *init = new RubyInit();
    init->initialize();
    rbinit = init;
}

/////////////
static VALUE nx_Qt_constant_missing(int argc, VALUE* argv, VALUE self)
{ return rbinit->Qt_constant_missing(argc, argv, self); }

static VALUE nx_Qt_method_missing(int argc, VALUE *argv, VALUE self)
{ return rbinit->Qt_method_missing(argc, argv, self); }

// for qApp 等全局变量
static VALUE nx_Qt_global_variable_get(ID id, VALUE *data, struct global_entry *entry)
{ return rbinit->Qt_global_variable_get(id, data, entry); }

static void nx_Qt_global_variable_set(VALUE value, ID id, VALUE *data, struct global_entry *entry)
{ rbinit->Qt_global_variable_set(value, id, data, entry); }

static VALUE nx_Qt_class_init(int argc, VALUE *argv, VALUE self)
{ return rbinit->Qt_class_init(argc, argv, self); }

static VALUE nx_Qt_class_dtor(VALUE id)
{ return rbinit->Qt_class_dtor(id); }

static VALUE nx_Qt_class_to_s(int argc, VALUE *argv, VALUE self)
{ return rbinit->Qt_class_to_s(argc, argv, self); }

static VALUE nx_Qt_class_const_missing(int argc, VALUE *argv, VALUE self)
{ return rbinit->Qt_class_const_missing(argc, argv, self); }

static VALUE nx_Qt_class_method_missing(int argc, VALUE *argv, VALUE self)
{ return rbinit->Qt_class_method_missing(argc, argv, self); }

static VALUE nx_Qt_class_singleton_method_missing(int argc, VALUE *argv, VALUE self)
{ return rbinit->Qt_class_singleton_method_missing(argc, argv, self); }


//////////////
void RubyInit::initialize()
{
    qInstallMessageHandler(myMessageOutput);
    gce = new CtrlEngine();
    
    static  VALUE cModuleQt = 0;
    cModuleQt = rb_define_module("Qt5");
    m_cModuleQt = cModuleQt;

    // 对所有的Qt5::someconst常量的调用注册
    rb_define_module_function(cModuleQt, "const_missing", FUNVAL nx_Qt_constant_missing, -1);
    // 对所有的Qt5::somefunc()函数的调用注册
    rb_define_module_function(cModuleQt, "method_missing", FUNVAL nx_Qt_method_missing, -1);
  
    // for qApp 类Qt全局变量
    rb_define_virtual_variable("$qApp", (VALUE (*)(ANYARGS)) nx_Qt_global_variable_get,
                               (void (*)(ANYARGS)) nx_Qt_global_variable_set);
}


/*
// 这应该叫统一的方法，获取qt class名称
// get_qt_class(VALUE self)
// 判断是否是qt类，如果不是，很可能是在Ruby中继承Qt类
// 如果是继承类，取到需要实例化的Qt类名
TODO 还需要考虑继承层次大于2的情况
@param self T_OBJECT | T_CLASS
 */
static QString get_qt_class(VALUE self)
{
    if (TYPE(self) == T_CLASS) {
        QString klass_name = rb_class2name(self);
        if (klass_name.startsWith("Qt5::Q")) return klass_name.split("::").at(1);

        // 如果是继承类，取到需要实例化的Qt类名
        VALUE pcls = RCLASS_SUPER(self);
        klass_name = rb_class2name(pcls);
        return klass_name.split("::").at(1);
    }
    // T_OBJECT
    else {
        QString klass_name = rb_class2name(RBASIC_CLASS(self));
        if (klass_name.startsWith("Qt5::Q")) return klass_name.split("::").at(1);

        // 如果是继承类，取到需要实例化的Qt类名
        VALUE pcls = RCLASS_SUPER(RBASIC_CLASS(self));
        klass_name = rb_class2name(pcls);
        return klass_name.split("::").at(1);
    }
}

static QString get_rb_class(VALUE self)
{
    if (TYPE(self) == T_CLASS) {
        QString klass_name = rb_class2name(self);
        if (klass_name.startsWith("Qt5::Q")) return QString();
        else return klass_name;
    }
    // T_OBJECT
    else {
        QString klass_name = rb_class2name(RBASIC_CLASS(self));
        if (klass_name.startsWith("Qt5::Q")) return QString();
        else return klass_name;
    }
}

VALUE RubyInit::Qt_constant_missing(int argc, VALUE* argv, VALUE self)
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

    // 可能是还没注册的类
    if (vname.at(0).toLatin1() == 'Q' && vname.at(1).isUpper()) {
        // TODO confirm it's a qt class, like by gce->hasClass();
        this->registClass(vname);
        VALUE cQtClass = this->getClass(vname);
        assert(cQtClass != Qnil);
        return cQtClass;
    }

    // vm exec
    int val = gce->vm_enum(vspace, vname);
    qDebug()<<"enum..."<<val;

    return INT2NUM(val);
    return Qnil;
}

VALUE RubyInit::Qt_method_missing(int argc, VALUE* argv, VALUE self)
{
    qDebug()<<"hehhe method missing."<< argc << argv << self;
    qDebug()<<argc<<TYPE(self); // == 3(T_MODULE)
    // RSTRING_PTR(argv[0]);
    for (int i = 0; i < argc; i ++) {
        qDebug()<<i<<TYPE(argv[i])<<MarshallRuby::VALUE2Variant(argv[i]);
    }
    // argv[0] is method symbol
    // argv[1] is method arg0
    // argv[2] is method arg1
    // ...
    QString method_name = rb_id2name(SYM2ID(argv[0]));
    qDebug()<<method_name;

    if (method_name.indexOf("connect") >= 0) {
        ConnectAny *conn = ConnectFactory::create(argc, argv, self);
        return Qnil;
    }
    /*
    // Qt5::connectrb    
    if (method_name == "connectrb") {
        qDebug()<<"got ittttttt."<<method_name;
        return x_Qt_connectrb(argc, argv, self);
    } 
    // Qt5::rbconnectrb
    else if (method_name == "rbconnectrb") {
        qDebug()<<"got ittttttt."<<method_name;
        return x_Qt_rbconnectrb(argc, argv, self);
    }
    // Qt5::rbdisconnectrb
    else if (method_name == "rbdisconnectrb") {
        qDebug()<<"got ittttttt."<<method_name;
        return x_Qt_rbdisconnectrb(argc, argv, self);
    }
    // Qt5::rbconnect    
    else if (method_name == "rbconnect") {
        qDebug()<<"got ittttttt."<<method_name;
        return x_Qt_rbconnect(argc, argv, self);
    }
    // Qt5::connect
    else if (method_name == "connect") {
        qDebug()<<"got ittttttt."<<method_name;
        return x_Qt_connect(argc, argv, self);        
    }
    */
    qDebug()<<"unimpled!!!";
    
    return Qnil;
}

VALUE RubyInit::Qt_global_variable_get(ID id, VALUE *data, struct global_entry *entry)
{
    // qDebug()<<id<<data<<entry;
    // qDebug()<<rb_id2name(id)<<qApp;
    QString vname = rb_id2name(id);

    if (vname == "$qApp") {
        void *v = qApp;
        if (v == NULL) return Qnil;
        // TODO 提高查找效率。
        return Qom::inst()->getObject(v);
    }
    qDebug()<<"undef variable:"<<vname;
    return INT2NUM(123456);
}

void RubyInit::Qt_global_variable_set(VALUE value, ID id, VALUE *data, struct global_entry *entry)
{
    qDebug()<<TYPE(value)<<id<<data<<entry;
    return;    
}

/*
  通过Qt类的初始化函数
  获取要实例化的类名，从staticMetaObject加载类信息，
  使用Qt的QMetaObject::newInstance创建新的实例对象。
  TODO:
  处理初始化时的参数。
 */
VALUE RubyInit::Qt_class_init(int argc, VALUE *argv, VALUE self)
{
    qDebug()<<argc<<TYPE(self);
    qDebug()<<rb_class2name(RBASIC_CLASS(self));
    qDebug()<<"has block:"<<rb_block_given_p();

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
        args << MarshallRuby::VALUE2Variant(argv[i]);
        qDebug()<<"i == "<< i << (i<argc) << (i>argc)<<MarshallRuby::VALUE2Variant(argv[i]);
    }

    CallArgument *cargs = new CallArgument(argc, argv, self, 0);
    qDebug()<<cargs->getArgs();
    void *jo = gce->vm_new(klass_name, args);
    qDebug()<<jo<<self<<rb_hash(self);    

    Qom::inst()->addObject(self, jo);

    VALUE free_proc = rb_proc_new(FUNVAL nx_Qt_class_dtor, 0);
    rb_define_finalizer(self, free_proc);

    // Qxxx.new() do {|i| }
    if (rb_block_given_p()) {
        rb_yield(self);
    }
    
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
  类实例的析构函数  
  TODO:
  使用通用方法之后，宏SAVE_XXX和GET_XXX就可以不需要了。
 */
VALUE RubyInit::Qt_class_dtor(VALUE id)
{
    VALUE os = rb_const_get(rb_cModule, rb_intern("ObjectSpace"));
    VALUE self = rb_funcall(os, rb_intern("_id2ref"), 1, id);

    // GET_CI0();
    // delete ci;

    // void* now, can not delete, need dtor and free
    void *qo = Qom::inst()->getObject(self);
    qDebug()<<qo<<rb_class2name(RBASIC_CLASS(self));

    return Qnil;
}

VALUE RubyInit::Qt_class_to_s(int argc, VALUE *argv, VALUE obj)
{
    qDebug()<<argc<<argv;
    qDebug()<<RBASIC_CLASS(obj);
    qDebug()<<rb_class2name(RBASIC_CLASS(obj));
    QString klass_name = QString(rb_class2name(RBASIC_CLASS(obj)));
    // klass_name = klass_name.split("::").at(1);
    klass_name = get_qt_class(obj);
    QString rbklass_name = get_rb_class(obj);
    void *ci = Qom::inst()->getObject(obj);

    QString jitstr = gce->vm_qdebug(ci, klass_name);
    
    QString stc; // stream container
    QDebug dm(&stc);
    // dm << "tsed:" << klass_name << "Obj:"<< obj << "Hash:" << rb_hash(obj) << "C:"<< ci;
    jitstr.replace("\"", "");
    QString inherit_prefix = rbklass_name.length() > 0 ? QString("%1 < ").arg(rbklass_name) : QString();
    // dm << "RBO:"<< obj << "[" << inhirent_prefix << jitstr << "]";
    dm << QString("RBO: %1 [ %2 %3 ]").arg(obj).arg(inherit_prefix).arg(jitstr);
    // 怎么能解引用呢 *ci, 当前void*无法解。
    // dm << "hehe has newline???"; // 这种方式要手动换行。

    // 简单方法，
    /*
    if (klass_name == "Qt5:QString") dm << *(YaQString*)ci;
    else if (klass_name == "Qt5::QUrl") dm << *(YaQUrl*)ci;

    if (klass_name == "Qt5::QWidget") {
        QWidget *w = (QWidget*)ci;
    }
    */
    
    // qDebug()<<stc;
    QString clean_stc = stc.mid(1, stc.length() - 3); // trim start/end's '"', and also a space char
    VALUE rv = rb_str_new2(clean_stc.toLatin1().data());
    // qDebug()<<klass_name<<rbklass_name<<("="+clean_stc+"=")<<("="+stc+"=");
    // exit(0);
    return rv;
}

/*
  解决类中的enum的处理
  TODO:
  使用FrontEngine查找
 */
VALUE RubyInit::Qt_class_const_missing(int argc, VALUE *argv, VALUE obj)
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
  stack structure:
  [0] => SYM function name
  [1] => arg0
  [2] => arg1
  [3] => arg2
  ...
 */
VALUE RubyInit::Qt_class_method_missing(int argc, VALUE *argv, VALUE self)
{
    void *jo = Qom::inst()->getObject(self);
    void *ci = jo;
    qDebug()<<ci<<argc;
    assert(ci != 0);
    QString klass_name = get_qt_class(self);
    QString rbklass_name = get_rb_class(self);
    QString method_name = QString(rb_id2name(SYM2ID(argv[0])));
    qDebug()<<"calling:"<<rbklass_name<<klass_name<<method_name<<argc<<(argc > 1);
    assert(argc >= 1);

    QVector<QVariant> args;
    args = MarshallRuby::ARGV2Variant(argc, argv, 1);
    qDebug()<<"callarg:"<<args;
    
    CallArgument *cargs = new CallArgument(argc, argv, self, 1);
    qDebug()<<"callarg:"<<cargs->getArgs();

    // fix try_convert(obj) → array or nil
    if (method_name == "to_ary") {
        return Qnil;
    }
    else if (method_name == "to_s") {
        return Qnil;
    }
    // for emit keyword, just as method 
    else if (method_name == "emit") {
        return argv[0];
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

    // for rbsignals
    QString signature = Qom::inst()->getSignature(rbklass_name, method_name);
    if (!signature.isEmpty()) {
        qDebug()<<"handling rbsignal..."<<signature;
        // 查找这个信号连接的所有slots列表，并依次执行。
        // QVector<Qom::RubySlot*> rbslots = Qom::inst()->getConnections(self, method_name);
        QVector<ConnectAny*> rbslots = Qom::inst()->getConnections5(self, method_name);
        qDebug()<<rbslots<<rbslots.count();
        if (rbslots.count() == 0) {
            qDebug()<<"no slot connected from signal:"<<rbklass_name<<method_name<<signature;
            return Qnil;
        }
        for (int i = 0; i < rbslots.count(); i ++) {
            auto slot = rbslots.at(i);
            // run slot now
            int rargc = argc - 1;
            VALUE *rargv = new VALUE[10];
            for (int i = 1; i < argc; i++) { rargv[i-1] = argv[i]; }
            qDebug()<<"invoking..."<<slot->toString();
            slot->acall(rargc, rargv, self);
        }
        return Qnil;
    }

    // others, call as a static member function
    qDebug()<<ci<<klass_name<<method_name<<args;
    QVariant gv = gce->vm_call(ci, klass_name, method_name, args);
    qDebug()<<"vv:"<<gv;
    // TODO 使用真实的返回值
    if (method_name == "height" || method_name == "width"
        || method_name == "x" || method_name == "y") {
        // return INT2NUM(gv.toInt());
    }
    if (gv.isValid()) {
        return gv.toULongLong();
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
VALUE RubyInit::Qt_class_singleton_method_missing(int argc, VALUE *argv, VALUE self)
{
    qDebug()<<argc;
    QString klass_name = get_qt_class(self);
    QString rbklass_name = get_rb_class(self);
    QString method_name = QString(rb_id2name(SYM2ID(argv[0])));
    qDebug()<<"calling:"<<rbklass_name<<klass_name<<method_name<<argc<<(argc > 1);        
    
    QVector<QVariant> args;
    args = MarshallRuby::ARGV2Variant(argc, argv, 1);

    // fix try_convert(obj) → array or nil
    if (method_name == "to_ary") {
        return Qnil;
    }
    else if (method_name == "to_s") {
        return Qnil;
    }
    // 处理signals方法
    else if (method_name == "signals") {
        // qDebug()<<args;
        Qom::inst()->addSignals(rbklass_name, args);
        return Qtrue;
    }
    
    QVariant gv = gce->vm_static_call(klass_name, method_name, args);
    qDebug()<<"vv:"<<gv;
    if (gv.isValid()) {
        return gv.toULongLong();
    }
    return Qnil;
}

bool RubyInit::registClass(QString klass)
{
    if (m_cModuleQt == 0) {
        qDebug()<<"uninit ModuleQt";
        return false;
    }
    if (isRegisted(klass)) return false;
        
    const char *cname = klass.toLatin1().data();
    VALUE module = this->m_cModuleQt;

    VALUE cQtClass = rb_define_class_under(module, cname, rb_cObject);
    rb_define_method(cQtClass, "initialize", (VALUE (*) (...)) nx_Qt_class_init, -1);
    rb_define_method(cQtClass, "to_s", (VALUE (*) (...)) nx_Qt_class_to_s, -1);
    rb_define_method(cQtClass, "method_missing", (VALUE (*) (...)) nx_Qt_class_method_missing, -1);
    rb_define_singleton_method(cQtClass, "const_missing",
                               (VALUE (*) (...)) nx_Qt_class_const_missing, -1);
    rb_define_singleton_method(cQtClass, "method_missing",             
                               (VALUE (*) (...)) nx_Qt_class_singleton_method_missing, -1);

    this->fixConflict(cQtClass, klass, "display");
    m_classes.insert(klass, cQtClass);
    return true;
}
