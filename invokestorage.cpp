#include "fix_clang_undef_eai.h"

#include <QDebug>
#include <QVariant>
#include <QThreadStorage>

#include <llvm/IR/Value.h>
#include <clang/AST/Expr.h>

#include "callargument.h"
#include "invokestorage.h"

int EvalType::id = qMetaTypeId<EvalType>();
EvalType::EvalType(clang::Expr *e, llvm::Value *v)
    : ve(e), vv(v)
{
}

void is_test1()
{
    EvalType v(0, 0);
    QVariant a = QVariant::fromValue(v);
    qDebug()<<a;
    EvalType v2 = a.value<EvalType>();
    qMetaTypeId<EvalType>();
    int i = 5;
    int id = qMetaTypeId<EvalType>();
    switch (i) {
    case 1:break;
        // case EvalType::id:  // 
        break;
    };
}




/*
  InvokeStorage用于把C++空间的变量作为参数映射到IR/JIT运行空间中。
  由于参数有不同的类型，所以每次调用都需要一个参数的存储空间。
  并且每次的存储空间也不相同。
  在第二版本的实现中，简单的使用了一个不同数据类型的稀疏矩阵方式，
  每次调用，在不同的矩阵点上放上不同的值，实现参数传递功能。
  这种方式，占用内存比较多。

  一种新的设计，满足以下特性：
  *) 使用更少的内存。
  *) 支持多线程使用。
  *) 减小复制，效率更高。

  InvokeStorage对象生存周期分析：
  由于它只在调用到IR/JIT运行空间有用，从绑定开始，一直到IR/JIT执行结束。
  如果在此存储结果的话，一直要到结果取出为止。
  
  实现，使用QThreadStorage实现线程相关的存储，使用QMetaType方式做数据的拷贝。
  参数类型分为scala类型与指针类型。
 */

class InvokeStorage3
{
public:
    // 参数序号
    enum {
        ARG1 = 1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9,
        RET = 126, // 存放返回值
        SRET = 127, // 存放sret结果
    };
    InvokeStorage3()
    {
    }
    ~InvokeStorage3()
    {
    }

    // get到封装类型
    const MetaTypeVariant *getFull(int which)
    {
        if ((which >= ARG1 && which <= ARG9) || which == RET || which == SRET) {
            MetaTypeVariant *pmtv = mcaches.localData().object(which);
            return pmtv;
        }
        return NULL;
    }
    // get到的是实际的数据地址
    void *get(int which)
    {
        if ((which >= ARG1 && which <= ARG9) || which == RET || which == SRET) {
            MetaTypeVariant *pmtv = mcaches.localData().object(which);
            return pmtv->get();
        }
        return NULL;
    }

    bool set(int which, const MetaTypeVariant &mtv)
    {
        if ((which >= ARG1 && which <= ARG9) || which == RET || which == SRET) {
            MetaTypeVariant *pmtv = new MetaTypeVariant(mtv);
            mcaches.localData().insert(which, pmtv);
            return true;
        }
        return false;
    }

    bool remove(int which)
    {
        if ((which >= ARG1 && which <= ARG9) || which == RET || which == SRET) {
            mcaches.localData().remove(which);
            return true;
        }
        return false;
    }

public:
    QThreadStorage<QCache<uchar, MetaTypeVariant> > mcaches;
};



















