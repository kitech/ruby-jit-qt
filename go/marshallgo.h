#ifndef MARSHALLRUBY_H
#define MARSHALLRUBY_H

#include <cassert>
#include <QtCore>

// #include "ruby_cxx.h"
#include "callargument.h"

/*
  负责ruby <=> qt的类型转换
 */

class MarshallGo
{
public:

    /*
      数字类型
      字符串类型
      对象类型
      数组类型(元素为数字，字符串，对象）
    */
    static QVariant VALUE2Variant(GoVar *v);

    // Range类型支持
    static QVector<QVariant> VALUE2Variant2(GoVar *v);

    static QVector<QVariant> ARGV2Variant(int argc, GoVarArray *argv, int start = 0);

    // @param v QMetaCallEvent->args()[n]
    static GoVar *Variant2VALUE(void *v, int type);

    
    /////// 使用MetaElem表示的Variant
    static QVector<QSharedPointer<MetaTypeVariant> > VALUE2MTVariant(GoVar *v);
    static QVector<QSharedPointer<MetaTypeVariant> > ARGV2MTVariant(int argc, GoVarArray *argv, int start = 1);
};


#endif /* MARSHALLRUBY_H */










