#ifndef ENTRY_H
#define ENTRY_H


#include <ruby.h>

VALUE x_Qt_meta_class_init_jit(int argc, VALUE *argv, VALUE self);
VALUE x_Qt_meta_class_init(int argc, VALUE *argv, VALUE self);
VALUE x_Qt_meta_class_const_missing(int argc, VALUE *argv, VALUE self);
VALUE x_Qt_meta_class_method_missing(int argc, VALUE *argv, VALUE self);
VALUE x_Qt_meta_class_singleton_method_missing(int argc, VALUE *argv, VALUE self);


#endif /* ENTRY_H */
