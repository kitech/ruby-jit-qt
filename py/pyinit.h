#ifndef PYINIT_H
#define PYINIT_H

#include <cassert>
#include <Python.h>

#include "extinit.h"

class CtrlEngine;

class PyInit : public ExtInit
{
public:
    PyInit() : ExtInit(){}
    virtual void initialize();

protected:
    CtrlEngine *gce = NULL;
    
public:
    PyObject* Qt_class_missing(int argc, void* argv, void* self);
    /*
    VALUE Qt_constant_missing(int argc, VALUE* argv, VALUE self);
    VALUE Qt_method_missing(int argc, VALUE* argv, VALUE self);
    VALUE Qt_global_variable_get(ID id, VALUE *data, struct global_entry *entry);
    void Qt_global_variable_set(VALUE value, ID id, VALUE *data, struct global_entry *entry);

    VALUE Qt_class_new(int argc, VALUE *argv, VALUE rb_cKlass);
    VALUE Qt_class_init(int argc, VALUE *argv, VALUE self);
    VALUE Qt_class_dtor(VALUE id);
    VALUE Qt_class_to_s(int argc, VALUE *argv, VALUE self);
    VALUE Qt_class_const_missing(int argc, VALUE *argv, VALUE self);
    VALUE Qt_class_method_missing(int argc, VALUE *argv, VALUE self);
    VALUE Qt_class_singleton_method_missing(int argc, VALUE *argv, VALUE self);
    */
public: // register
    bool isRegisted(QString klass)
    {
        // return m_classes.contains(klass);
    }

    /*
    VALUE getClass(QString klass)
    {
        assert(isRegisted(klass));
        VALUE cQtClass = Qnil;
        cQtClass = m_classes.value(klass);
        return cQtClass;
    }
    */
    
    bool registClass(QString klass);
    bool unregistClass(QString klass)
    {
        if (isRegisted(klass)) {
            /*
            VALUE cQtClass = m_classes.value(klass);
            ID cid = rb_to_id(cQtClass);

            rb_undef_method(cQtClass, "method_missing");
            rb_undef_method(cQtClass, "to_s");
            rb_undef_method(cQtClass, "initialize");
            
            rb_undef(cQtClass, cid);
            */
            return true;
        }
        return true;
    }

    /*
    bool fixConflict(VALUE cQtClass, QString klass, QString method)
    {
        if (klass == "QLCDNumber") {
            rb_undef_method(cQtClass, method.toLatin1().data());
        }
        return true;
    }
    */
    
protected:
    // QHash<QString, VALUE> m_classes;
    // VALUE m_cModuleQt = 0;
public:
    QHash<QString, PyTypeObject*> m_classes;
    PyObject* m_cModuleQt = 0;
};


#endif /* PYINIT_H */
