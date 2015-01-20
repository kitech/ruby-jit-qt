#ifndef MARSHALLRUBY_H
#define MARSHALLRUBY_H

#include <cassert>
#include <QtCore>

#include "ruby_cxx.h"

/*
  负责ruby <=> qt的类型转换
 */

class MarshallRuby
{
public:

    /*
      数字类型
      字符串类型
      对象类型
      数组类型(元素为数字，字符串，对象）
    */
    static QVariant VALUE2Variant(VALUE v);

    // Range类型支持
    static QVector<QVariant> VALUE2Variant2(VALUE v);

    static QVector<QVariant> ARGV2Variant(int argc, VALUE *argv, int start = 0);

    // @param v QMetaCallEvent->args()[n]
    static VALUE Variant2VALUE(void *v, int type);
};


#endif /* MARSHALLRUBY_H */
