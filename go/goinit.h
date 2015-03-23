#ifndef GOINIT_H
#define GOINIT_H

#include "extinit.h"

class GoInit : public ExtInit
{
public:
    GoInit() : ExtInit()
    {
    }

    virtual void initialize();

protected:
    class CtrlEngine *gce = NULL;
    
public:
    void *Qt_class_new(int argc, void *argv, void *klass);
    void *Qt_class_method_missing(int argc, void *argv, void *obj);
    /*
    VALUE Qt_class_init(int argc, VALUE *argv, VALUE self);
    VALUE Qt_class_dtor(VALUE id);
    VALUE Qt_class_to_s(int argc, VALUE *argv, VALUE self);
    VALUE Qt_class_const_missing(int argc, VALUE *argv, VALUE self);
    VALUE Qt_class_method_missing(int argc, VALUE *argv, VALUE self);
    VALUE Qt_class_singleton_method_missing(int argc, VALUE *argv, VALUE self);
    */
public: // register
    
protected:
    
};


#endif /* GOINIT_H */
