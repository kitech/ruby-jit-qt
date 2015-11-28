
#include "qtobjectmanager.h"

#include "go/api/src/dynamic/vtype.h"
#include "marshallgo.h"

/*
  数字类型
  字符串类型
  对象类型
  数组类型(元素为数字，字符串，对象）
 */
QVariant MarshallGo::VALUE2Variant(GoVar *v)
{
    /*
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
    
    return (rv);
    */
}

// Range类型支持
QVector<QVariant> MarshallGo::VALUE2Variant2(GoVar *v)
{
    /*
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
    return (rvs);
    */
}

QVector<QVariant> MarshallGo::ARGV2Variant(int argc, GoVarArray *argv, int start)
{
    /*
    QVector<QVariant> args;
    for (int i = start; i < argc; i ++) {
        // if (i == 0) continue; // for ctor 0 is arg0
        if (i >= argc) break;
        qDebug()<<"i == "<< i << (i<argc) << (i>argc);
        QVector<QVariant> targs = VALUE2Variant2(argv[i]);
        for (auto v: targs) args << v;
        qDebug()<<"i == "<< i << (i<argc) << (i>argc)<<targs;
    }

    return (args);
    */
}

// static
GoVar *MarshallGo::Variant2VALUE(void *v, int type)
{
    /*
    VALUE obj = Qom::inst()->getObject(v);
    if (obj != 0) {
        return obj;
    }

    int pty = type;
    void *arg = v;

    VALUE rv = Qnil;
    qDebug()<<arg<<pty<<QString(QMetaType::typeName(pty));
    switch(pty) {
    case QMetaType::QString: {
        QString s(*(QString*)arg);
        qDebug()<<s;
        qDebug()<<Qom::inst()->getObject(arg)<<Qnil;
        rv = rb_str_new2(((QString*)arg)->toLatin1().data());
    } break;
    default:
        qDebug()<<"unknown type:"<<pty<<QString(QMetaType::typeName(pty));
        break;
    }

    return rv;
    */
}

/////////////////////
// static
QVector<QSharedPointer<MetaTypeVariant> >
MarshallGo::VALUE2MTVariant(GoVar *v)
{
    /*
    QVector<QSharedPointer<MetaTypeVariant> > rvs(1);
    QSharedPointer<MetaTypeVariant> rv;
    QString str, str2;
    void *ci = NULL;
    QObject *obj = NULL;

    VALUE v1, v2, v3;
    
    switch (TYPE(v)) {
    case T_NONE:  rv = QSharedPointer<MetaTypeVariant>(new MetaTypeVariant()); break;
    case T_FIXNUM: {
        QVariant num = (int)FIX2INT(v);
        rv = QSharedPointer<MetaTypeVariant>(new MetaTypeVariant(QMetaType::Int, &num));
    }; break;
    case T_STRING: {
        QVariant str = QString(RSTRING_PTR(v));
        rv = QSharedPointer<MetaTypeVariant>(new MetaTypeVariant(QMetaType::QString, &str));
    }; break;
    case T_FLOAT: {
        QVariant num = RFLOAT_VALUE(v);
        rv = QSharedPointer<MetaTypeVariant>(new MetaTypeVariant(QMetaType::Double, &num));
    }; break;
    case T_NIL: {
        QVariant  num = 0;
        rv = QSharedPointer<MetaTypeVariant>(new MetaTypeVariant(QMetaType::Int, &num));
    }; break;
    case T_TRUE: {
        QVariant ok = true;
        rv = QSharedPointer<MetaTypeVariant>(new MetaTypeVariant(QMetaType::Bool, &ok));
    }; break;
    case T_FALSE: {
        QVariant ok = false;
        rv = QSharedPointer<MetaTypeVariant>(new MetaTypeVariant(QMetaType::Bool, &ok));
    }; break;
    case T_OBJECT: {
        str = QString(rb_class2name(RBASIC_CLASS(v)));
        ci = Qom::inst()->getObject(v);
        // obj = dynamic_cast<QObject*>(ci);
        qDebug()<<"unimpl VALUE:"<<str<<ci<<obj;
        // rv = QVariant(QMetaType::VoidStar, ci);
        // rv = QVariant::fromValue(ci);
        QVariant vrv = QVariant::fromValue(ci);
        rv = QSharedPointer<MetaTypeVariant>(new MetaTypeVariant(QMetaType::VoidStar, &vrv));
    }; break;
    case T_ARRAY: {
        QStringList ary;
        // ary << "a123" << "b3456";
        qDebug()<<RARRAY_LEN(v)<<QT_VERSION;
        for (int i = 0; i < RARRAY_LEN(v); i++) {
            // FIXME: 也可能是对象数组，如Qt5::QString，但toString方法不好用。
            // FIXME: 如，[Qt5::QApplication.translate("MainWindow", "acbc", nil), "efgggggg", "hijjjjjjjj"]
            ary << VALUE2Variant(rb_ary_entry(v, i)).toString();
        }
        // rv = QVariant(ary);
        QVariant vary(ary);
        rv = QSharedPointer<MetaTypeVariant>(new MetaTypeVariant(QMetaType::QStringList, &vary));
    }; break;
    case T_STRUCT: { // for ruby range
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

            // rv = QVariant(FIX2INT(v1));
            // rvs.append(QVariant(FIX2INT(v2)));
            QVariant num1 = FIX2INT(v1);
            QVariant num2 = FIX2INT(v2);
            rv = QSharedPointer<MetaTypeVariant>(new MetaTypeVariant(QMetaType::Int, &num1));
            QSharedPointer<MetaTypeVariant> mtv(new MetaTypeVariant(QMetaType::Int, &num2));
            rvs.append(mtv);
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
    return (rvs);
    */
}

// static
QVector<QSharedPointer<MetaTypeVariant> >
MarshallGo::ARGV2MTVariant(int argc, GoVarArray *argv, int start)
{
    /*
    QVector<QSharedPointer<MetaTypeVariant> > args;
    for (int i = start; i < argc; i ++) {
        if (i >= argc) break;
        qDebug()<<"i == "<< i << (i<argc) << (i>argc);
        QVector<QSharedPointer<MetaTypeVariant> > targs = VALUE2MTVariant(argv[i]);
        for (auto &v: targs) args << v;
        qDebug()<<"i == "<< i << (i<argc) << (i>argc)<<targs;
    }
    
    return (args);
    */
}

