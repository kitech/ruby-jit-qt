#ifndef ENTRY_H
#define ENTRY_H

#include "fix_clang_undef_eai.h"
#include <ruby.hpp>

/*
  带meta的函数，是使用Qt moc方式实现的
  带jit后缀的，是使用半jit方式的。
  带jit后缀，不带meta的，是使用更接近jit的动态生成Qt库代码的方式，不需要预先生成YaQxxx类了。
  x_Qt_const_missing
  x_Qt_method_missing

  x_Qt_class_singleton_const_missing  # 类常量，不需要实例访问的常量
  x_Qt_class_const_missing            # 实例常量
  x_Qt_class_singleton_method_missing #类方法，不需要实例访问的常量，即静态方法
  x_Qt_class_method_missing


  x_Qt_class_const_missing_jit
  x_Qt_class_method_missing_jit

  x_Qt_class_const_missing_meta
  x_Qt_class_method_missing_meta
 */

enum {
    QT_BINDING_MOC, 
    QT_BINDING_YAJIT,
    QT_BINDING_JIT
};


// 不同方式的公用方法
VALUE x_Qt_meta_class_to_s(int argc, VALUE *argv, VALUE obj);

// 使用Qt moc方式
VALUE x_Qt_meta_class_init(int argc, VALUE *argv, VALUE self);
VALUE x_Qt_meta_class_const_missing(int argc, VALUE *argv, VALUE self);
VALUE x_Qt_meta_class_method_missing(int argc, VALUE *argv, VALUE self);
VALUE x_Qt_meta_class_singleton_method_missing(int argc, VALUE *argv, VALUE self);

// 使用预生成YaQxxx类方式
VALUE x_Qt_meta_class_init_jit(int argc, VALUE *argv, VALUE self);
VALUE x_Qt_meta_class_method_missing_jit(int argc, VALUE *argv, VALUE self);

// 使用即时编译方式
VALUE x_Qt_class_init_jit(int argc, VALUE *argv, VALUE self);
VALUE x_Qt_class_method_missing_jit(int argc, VALUE *argv, VALUE self);
VALUE x_Qt_class_singleton_method_missing_jit(int argc, VALUE *argv, VALUE self);
VALUE x_Qt_class_const_missing_jit(int argc, VALUE *argv, VALUE obj);

// Qt全局变量
VALUE x_Qt_global_variable_get(ID id, VALUE *data, struct global_entry *entry);
void x_Qt_global_variable_set(VALUE value, ID id, VALUE *data, struct global_entry *entry);


#endif /* ENTRY_H */
