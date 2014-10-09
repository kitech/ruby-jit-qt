#ifndef QTRUBY_H
#define QTRUBY_H

/*
  注册Qt类到ruby的进程空间。
 */

#include "ruby.hpp"

extern "C" {
    int register_qtruby_classes(VALUE module);
};

#endif /* QTRUBY_H */
