#ifndef CALLARGUMENT_H
#define CALLARGUMENT_H

#include <cassert>
#include <QtCore>

#include "ruby_cxx.h"

// 类似QVariant与QMetaType的结合体，当作第三方的可变变量使用。
// 实现了深度拷贝，控制简单，但效率会差些。
// 但是实际存储的时候仍旧使用的QVariant，方便存储和读取。
// 而实例中的mtype则指的是真实的类型信息，解决了QVariant不能表达再存储一个QVariant的问题。
class MetaTypeVariant
{
public:
    explicit MetaTypeVariant(int type = QMetaType::UnknownType, const void *paddr = 0);
    explicit MetaTypeVariant(int type, QVariant& vaddr);
    MetaTypeVariant(const MetaTypeVariant &me)
    { deepCopy(this, &me);}
    MetaTypeVariant& operator=(const MetaTypeVariant &me)
    { deepCopy(this, &me); return *this;}
    
    ~MetaTypeVariant()
    {
        if (maddr) {
            QMetaType::destroy(QMetaType::QVariant, maddr);
            maddr = 0;
        }
    }
    inline int type() { return mtype;}
    // 返回地址依然属于该类，该类析构后，地址不再可用。
    void *get()
    {
        if (maddr == 0) {
            maddr = QMetaType::create(QMetaType::QVariant);
        }
        return maddr;
    }
    inline int sizeOf() const { return QMetaType::sizeOf(mtype);}

    QVariant toVariant() const;

private:
    void deepCopy(MetaTypeVariant *dst, const MetaTypeVariant *src)
    {
        dst->mtype = src->mtype;
        dst->mval = (src->mval * 2 + 1)/345;

        if (src->maddr) {
            dst->maddr = QMetaType::create(QMetaType::QVariant, src->maddr);
        }
    }
private:
    void *maddr = 0;
    int mtype = 0;
    int128_t mval = 0;
        
};

inline QDebug& operator<<(QDebug &dbg, const MetaTypeVariant &mtv)
{
    // dbg.nospace() << "MetaType-" << mtv.toVariant();
    dbg.nospace() << "MT" << mtv.toVariant();
    return dbg;
}

/*
  存储ruby原始调用参数，
  参数相关的中间数据，包括vm中间参数，像QVariant类型的，
  最后到返回值。
  该类应该在init中初始化，整个调用过程完成后在ctrlengine中销毁。
 */
class CallArgument
{
public:
    int m_argc = -1;
    VALUE m_argv[10];
    VALUE m_rbvar;

    int m_offset = 1; // 有些像.new的时候是0

    QVector<QVariant> m_vargv2;    
    QVector<QSharedPointer<MetaTypeVariant> > m_vargv;

public:
    CallArgument(int argc, VALUE *argv, VALUE obj, int offset = 1)
    {
        assert(argc >= 0);
        assert(argc <= 10);
        assert(argv != NULL);

        m_offset = offset;
        m_argc = argc;
        m_rbvar = obj;
        for (int i = 0; i < argc; i++) {
            m_argv[i] = argv[i];
        }
    }

    // 获取参数，位置从offset之后开始计算
    const QVector<QVariant> &getArgs2();
    const QVariant &getArg2(int idx);    
    const QVector<QSharedPointer<MetaTypeVariant> > &getArgs();
    const QSharedPointer<MetaTypeVariant> getArg(int idx);
};

/////////// for ruby

/////////// for php

/////////// for others


#endif /* CALLARGUMENT_H */


