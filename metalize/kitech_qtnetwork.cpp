#include "utils.h"
#include "qom.h"
#include "entry.h"

#include "kitech_qtnetwork.h"
#include "metar_classes_qtnetwork.h"

/*
static VALUE x_QHostAddress_destructor(VALUE id)
{
    VALUE os = rb_const_get(rb_cModule, rb_intern("ObjectSpace"));
    VALUE self = rb_funcall(os, rb_intern("_id2ref"), 1, id);
    // qDebug()<<"gcedddddddddddd"<<&id;

    GET_CI3(QHostAddress);
    delete ci;

    return Qnil;
}

static VALUE x_QHostAddress_init(VALUE self)
{
    yQHostAddress *o = new yQHostAddress();
    SAVE_CI2(QHostAddress, o);
    // qDebug()<<rb_hash(self);

    VALUE free_proc = rb_proc_new(FUNVAL x_QHostAddress_destructor, 0);
    rb_define_finalizer(self, free_proc);

    return self;
}

static VALUE x_QTcpSocket_destructor(VALUE id)
{
    VALUE os = rb_const_get(rb_cModule, rb_intern("ObjectSpace"));
    VALUE self = rb_funcall(os, rb_intern("_id2ref"), 1, id);

    GET_CI3(QTcpSocket);
    delete ci;

    return Qnil;
}

static VALUE x_QTcpSocket_init(VALUE self)
{
    yQTcpSocket *sock = new yQTcpSocket;
    SAVE_CI2(QTcpSocket, sock);

    VALUE free_proc = rb_proc_new(FUNVAL x_QTcpSocket_destructor, 0);
    rb_define_finalizer(self, free_proc);

    return self;
}

static VALUE x_QTcpServer_destructor(VALUE id)
{
    VALUE os = rb_const_get(rb_cModule, rb_intern("ObjectSpace"));
    VALUE self = rb_funcall(os, rb_intern("_id2ref"), 1, id);

    GET_CI0();
    delete ci;
    
    return Qnil;
}

static VALUE x_QTcpServer_init(VALUE self)
{
    yQTcpServer *io = new yQTcpServer;
    SAVE_CI0(io);

    VALUE free_proc = rb_proc_new(FUNVAL x_QTcpServer_destructor, 0);
    rb_define_finalizer(self, free_proc);

    return self;
}
*/


extern "C" {
    VALUE rcQTcpSocket;
    VALUE rcQHostAddress;
    VALUE rcQTcpServer;

    int register_qtnetwork_methods(VALUE module)
    {
        rcQTcpSocket = rb_define_class_under(module, "QTcpSocket", rb_cObject);
        rb_define_method(rcQTcpSocket, "initialize", FUNVAL x_Qt_meta_class_init, -1);
        rb_define_method(rcQTcpSocket, "method_missing", FUNVAL x_Qt_meta_class_method_missing, -1);

        rcQHostAddress = rb_define_class_under(module, "QHostAddress", rb_cObject);
        rb_define_method(rcQHostAddress, "initialize", FUNVAL x_Qt_meta_class_init, -1);
        rb_define_method(rcQHostAddress, "method_missing", FUNVAL x_Qt_meta_class_method_missing, -1);
        // rb_define_method(rcQHostAddress, "const_missing", FUNVAL x_Qt_meta_class_const_missing, -1); // do northing
        // rb_define_method(rcQHostAddress, "self.const_missing", FUNVAL x_Qt_meta_class_const_missing, -1); // do northing
        rb_define_singleton_method(rcQHostAddress, "const_missing", FUNVAL x_Qt_meta_class_const_missing, -1);
        // rb_define_const(rcQHostAddress, "Any", INT2NUM(QHostAddress::Any));
        rb_define_singleton_method(rcQHostAddress, "method_missing",
                                   FUNVAL x_Qt_meta_class_singleton_method_missing, -1);

        rcQTcpServer = rb_define_class_under(module, "QTcpServer", rb_cObject);
        rb_define_method(rcQTcpServer, "initialize", FUNVAL x_Qt_meta_class_init, -1);
        rb_define_method(rcQTcpServer, "method_missing", FUNVAL x_Qt_meta_class_method_missing, -1);
        rb_define_singleton_method(rcQTcpServer, "const_missing",
                                   FUNVAL x_Qt_meta_class_const_missing, -1);
        rb_define_singleton_method(rcQTcpServer, "method_missing",
                                   FUNVAL x_Qt_meta_class_singleton_method_missing, -1);
        return 0;
    }
};


