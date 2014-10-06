
classes = [
           'QByteArray',
           'QThread',
           'QProcess',
           'QUrl',
           'QTimer',
           'QDateTime',
           'QJsonObject',
           'QJsonValue',
           'QJsonArray',
           'QStringList',
           'QUuid',
           'QFile',
           'QDir',
           'QTcpSocket',
           'QUdpSocket',
           'QTcpServer',
           'QNetworkAccessManager',
           'QWebSocket',
           # 'QWebSocketServer',
           'QMainWindow',
           'QWidget',
           'QPixmap',
           'QImage',
          ];

out = ''
gname = ''
init = "static void init_these_classes() {\n"

classes.each { |c|
  # puts c
  gname << "VALUE c#{c}; \n"

  dtor = "static VALUE x_#{c}_destructor(VALUE id)
{
    // qDebug()<<__FUNCTION__;

    VALUE os = rb_const_get(rb_cModule, rb_intern(\"ObjectSpace\"));
    VALUE self = rb_funcall(os, rb_intern(\"_id2ref\"), 1, id);
    // VALUE self = ID2SYM(id);
    GET_CI2(#{c});
    qDebug()<<__FUNCTION__<<ci<<NUM2ULONG(id)<<QString(\"%1\").arg(TYPE(self));
    delete ci;

    return Qnil;
}
";

  # puts dtor

  ctor = "static VALUE x_#{c}_init(VALUE self)
{ 
    #{c} *ci = new #{c}();
    SAVE_CI2(#{c}, ci);
    VALUE free_proc = rb_proc_new((VALUE (*) (...)) x_#{c}_destructor, 0);
    rb_define_finalizer(self, free_proc);
    return self;
}";
  # puts ctor;

  toqor = "static void *x_#{c}_to_q(VALUE self)
{
    GET_CI2(#{c});

    return Qnil;    
}";

  init << "    c#{c} = rb_define_class(\"#{c}\", rb_cObject);
    rb_define_method(c#{c}, \"initialize\", FUNVAL x_#{c}_init, 0);
\n";

  out << dtor << "\n" << ctor << "\n"
};

init << "}";

puts  "\n\n" << gname << "\n\n"
puts out  << "\n\n"
puts init  << "\n\n"


