#ifndef CALLARGUMENT_H
#define CALLARGUMENT_H

#include <cassert>
#include <QtCore>

#include "ruby_cxx.h"

/*
  存储ruby原始调用参数，
  参数相关的中间数据，包括vm中间参数，像QVariant类型的，
  最后到返回值
 */
class CallArgument
{
public:
    int m_argc = -1;
    VALUE m_argv[10];
    VALUE m_rbvar;

    static constexpr int m_offset = 1;
    QVector<QVariant> m_vargv;

public:
    CallArgument(int argc, VALUE *argv, VALUE obj)
    {
        assert(argc >= 0);
        assert(argc <= 10);
        assert(argv != NULL);
        
        m_argc = argc;
        m_rbvar = obj;
        for (int i = 0; i < argc; i++) {
            m_argv[i] = argv[i];
        }
    }

    // 获取参数，位置从offset之后开始计算
    const QVector<QVariant> &getArgs();
    const QVariant &getArg(int idx);
};

#endif /* CALLARGUMENT_H */


