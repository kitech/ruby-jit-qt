
#include "qtobjectmanager.h"

#include "marshallruby.h"

/*
  数字类型
  字符串类型
  对象类型
  数组类型(元素为数字，字符串，对象）
 */
QVariant MarshallRuby::VALUE2Variant(VALUE v)
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
        ci = Qom::inst()->getObject(v);
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
QVector<QVariant> MarshallRuby::VALUE2Variant2(VALUE v)
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
        ci = Qom::inst()->getObject(v);
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
            // FIXME: 也可能是对象数组，如Qt5::QString，但toString方法不好用。
            // FIXME: 如，[Qt5::QApplication.translate("MainWindow", "acbc", nil), "efgggggg", "hijjjjjjjj"]
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

QVector<QVariant> MarshallRuby::ARGV2Variant(int argc, VALUE *argv, int start)
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

