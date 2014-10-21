#ifndef OPERATORENGINE_H
#define OPERATORENGINE_H

// 生成调用相关的粘全代码，
// 这部分生成的代码，用于调用由complierengine的Qt代码
// 其功能类似现在的clvm_operator类的功能。
// 希望能更简洁和清晰全面些。
// 生成的函数是一个类似lamba函数，没有参数，而真正调用需要的参数转换成该函数的临时变量。

#include <QtCore>

namespace llvm {
    class Module;
};

class OperatorEngine
{
public:
    OperatorEngine();
    void init();

public:
    llvm::Module *mtmod = NULL;

public:
    // 类似std::bind
    QString bind(llvm::Module *mod, QString symbol, void *kthis, QVector<QVariant> uargs
                 , QVector<QVariant> dargs);
    
};

#endif /* OPERATORENGINE_H */










