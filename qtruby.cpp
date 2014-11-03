#include "entry.h"
#include "qtruby.h"

extern "C" {

    // 注意这个符号""#klass => """QUrl"
    // <===> 
    // cQUrl = rb_define_class_under(module, "QUrl", rb_cObject);
    // rb_define_method(cQUrl, "initialize", FUNVAL x_Qt_meta_class_init_jit, -1);
    // rb_define_method(cQUrl, "method_missing", FUNVAL x_Qt_meta_class_method_missing_jit, -1);
    // rb_define_singleton_method(cQUrl, "const_missing", FUNVAL x_Qt_meta_class_const_missing_jit, -1);

#define RQCLASS_REGISTER(klass)                                         \
    static VALUE c##klass = rb_define_class_under(module, ""#klass, rb_cObject); \
    rb_define_method(c##klass, "initialize", (VALUE (*) (...)) x_Qt_class_init_jit, -1); \
    rb_define_method(c##klass, "to_s", (VALUE (*) (...)) x_Qt_meta_class_to_s, -1); \
    rb_define_method(c##klass, "method_missing", (VALUE (*) (...)) x_Qt_class_method_missing_jit, -1); \
    rb_define_singleton_method(c##klass, "const_missing",  (VALUE (*) (...)) x_Qt_class_const_missing_jit, -1);

#define RQCLASS_REGISTER_YAJIT(klass)                                         \
    static VALUE c##klass = rb_define_class_under(module, ""#klass, rb_cObject); \
    rb_define_method(c##klass, "initialize", (VALUE (*) (...)) x_Qt_meta_class_init_jit, -1); \
    rb_define_method(c##klass, "to_s", (VALUE (*) (...)) x_Qt_meta_class_to_s, -1); \
    rb_define_method(c##klass, "method_missing", (VALUE (*) (...)) x_Qt_meta_class_method_missing_jit, -1);

    
    // VALUE cQObject;
    // VALUE cQString;
    // VALUE cQCoreApplication;
    // VALUE cQByteArray;
    // VALUE cQUrl;

    int register_qtruby_classes(VALUE module)
    {
        // 所有Qt类的注释
        #include "metalize/qtruby_auto_body.cpp"
        // format: RQCLASS_REGISTER(QXxxxx);

        // cQObject = rb_define_class_under(module, "QObject", rb_cObject);
        // rb_define_method(cQObject, "initialize", FUNVAL x_Qt_meta_class_init, -1);
        // rb_define_method(cQObject, "method_missing", FUNVAL x_Qt_meta_class_method_missing, -1);
        // // like class enum
        // rb_define_singleton_method(cQObject, "const_missing", FUNVAL x_Qt_meta_class_const_missing, -1);
        // // like static method
        // rb_define_singleton_method(cQObject, "method_missing", 
        //                            FUNVAL x_Qt_meta_class_singleton_method_missing, -1);


        // // TODO maybe this can also automatical run
        // cQString = rb_define_class_under(module, "QString", rb_cObject);
        // // rb_define_method(cQString, "initialize", (VALUE (*) (...)) x_QString_init, 0);
        // rb_define_method(cQString, "initialize", FUNVAL x_Qt_meta_class_init_jit, -1);
        // // rb_define_method(cQString, "append", (VALUE (*) (...)) x_QString_append, 1);
        // // rb_define_method(cQString, "to_s", (VALUE (*) (...)) x_QString_to_s, 0);
        // // rb_define_method(cQString, "method_missing", FUNVAL x_QString_method_missing, -1);
        // rb_define_method(cQString, "method_missing", FUNVAL x_Qt_meta_class_method_missing_jit, -1);
        // rb_define_method(cQString, "to_s", FUNVAL x_QString_to_s, 0);

        // cQByteArray = rb_define_class_under(module, "QByteArray", rb_cObject);
        // rb_define_method(cQByteArray, "initialize", FUNVAL x_Qt_meta_class_init_jit, -1);
        // rb_define_method(cQByteArray, "method_missing", FUNVAL x_Qt_meta_class_method_missing_jit, -1);

        // RQCLASS_REGISTER(QUrl);
        // // cQUrl = rb_define_class_under(module, "QUrl", rb_cObject);
        // // rb_define_method(cQUrl, "initialize", FUNVAL x_Qt_meta_class_init_jit, -1);
        // // rb_define_method(cQUrl, "method_missing", FUNVAL x_Qt_meta_class_method_missing_jit, -1);

        // cQCoreApplication = rb_define_class_under(module, "QCoreApplication", rb_cObject);
        // rb_define_method(cQCoreApplication, "initialize", FUNVAL x_Qt_meta_class_init, 0);
        // // rb_define_method(cQCoreApplication, "exec", FUNVAL x_QCoreApplication_exec, 0);
        // // rb_define_method(cQCoreApplication, "quit", FUNVAL x_QCoreApplication_quit, 0);
        
        return 0;
    }
};
