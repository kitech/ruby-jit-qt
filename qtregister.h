#ifndef QTREGISTER_H
#define QTREGISTER_H

#include <cassert>
#include <QtCore>

#include "macrolib.h"
#include "ruby_cxx.h"
#include "entry.h"

class QtRegister;
static QtRegister *qrti = NULL;

/*
  管理Qt类在ruby中的注册
  auto rger = QtRegister::inst();
 */
class QtRegister : public Singleton<QtRegister>
{
public:
    friend class Singleton;
    
public:
    QtRegister() { qrti = this; }
    void setModuleQt(VALUE cMoudleQt) { m_cModuleQt = cMoudleQt;}
    
    bool isRegisted(QString klass)
    {
        return m_classes.contains(klass);
    }
    
    VALUE getClass(QString klass)
    {
        assert(isRegisted(klass));
        VALUE cQtClass = Qnil;
        cQtClass = m_classes.value(klass);
        return cQtClass;
    }
    
    bool registClass(QString klass)
    {
        if (m_cModuleQt == 0) {
            qDebug()<<"uninit ModuleQt";
            return false;
        }
        if (isRegisted(klass)) return false;
        
        const char *cname = klass.toLatin1().data();
        VALUE module = this->m_cModuleQt;

        VALUE cQtClass = rb_define_class_under(module, cname, rb_cObject);
        rb_define_method(cQtClass, "initialize", (VALUE (*) (...)) x_Qt_class_init_jit, -1);
        rb_define_method(cQtClass, "to_s", (VALUE (*) (...)) x_Qt_meta_class_to_s, -1);
        rb_define_method(cQtClass, "method_missing", (VALUE (*) (...)) x_Qt_class_method_missing_jit, -1);
        rb_define_singleton_method(cQtClass, "const_missing",
                                   (VALUE (*) (...)) x_Qt_class_const_missing_jit, -1);
        rb_define_singleton_method(cQtClass, "method_missing",             
                                   (VALUE (*) (...)) x_Qt_class_singleton_method_missing_jit, -1);

        this->fixConflict(cQtClass, klass, "display");
        m_classes.insert(klass, cQtClass);
        return true;
    }

    bool unregistClass(QString klass)
    {
        if (isRegisted(klass)) {
            VALUE cQtClass = m_classes.value(klass);
            ID cid = rb_to_id(cQtClass);

            rb_undef_method(cQtClass, "method_missing");
            rb_undef_method(cQtClass, "to_s");
            rb_undef_method(cQtClass, "initialize");
            
            rb_undef(cQtClass, cid);
            return true;
        }
        return true;
    }

    bool fixConflict(VALUE cQtClass, QString klass, QString method)
    {
        if (klass == "QLCDNumber") {
            rb_undef_method(cQtClass, method.toLatin1().data());
        }
        return true;
    }
    
protected:
    QHash<QString, VALUE> m_classes;
    VALUE m_cModuleQt = 0;
};


#endif /* QTREGISTER_H */
