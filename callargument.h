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

    // 实现了深度拷贝，控制简单，但效率会差些。
    class MetaElem
    {
    public:
        MetaElem(int type = QMetaType::UnknownType, const void *paddr = 0)
            : mtype(type)
        {
            if (paddr) {
                maddr = QMetaType::create(mtype, paddr);
            }
        }
        MetaElem(MetaElem &me)
        {
            mtype = me.mtype;
            mval = (me.mval * 2 + 1)/345;

            if (me.maddr) {
                maddr = QMetaType::create(mtype, me.maddr);                
            }
        }
        ~MetaElem()
        {
            if (maddr) {
                QMetaType::destroy(mtype, maddr);
                maddr = 0;
            }
        }
        inline int type() { return mtype;}
        // 返回地址依然属于该类，该类析构后，地址不再可用。
        void *get()
        {
            if (maddr == 0) {
                maddr = QMetaType::create(mtype);
            }
            return maddr;
        }
        inline int sizeOf() { return QMetaType::sizeOf(mtype);}

        QVariant toVariant()
        {
            QDataStream stm;
            bool bret = QMetaType::save(stm, mtype, get());
            return QVariant(stm);
        }
        
    private:
        void *maddr = 0;
        int mtype = 0;
        int128_t mval = 0;
        
    };
    QVector<QVariant> m_vargv;
    QVector<MetaElem> m_vargElems;

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


