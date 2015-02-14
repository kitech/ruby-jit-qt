#ifndef OPERATORENGINE_H
#define OPERATORENGINE_H

// 生成调用相关的粘全代码，
// 这部分生成的代码，用于调用由complierengine的Qt代码
// 其功能类似现在的clvm_operator类的功能。
// 希望能更简洁和清晰全面些。
// 生成的函数是一个类似lamba函数，没有参数，而真正调用需要的参数转换成该函数的临时变量。

#include <QtCore>

#include <llvm/IR/IRBuilder.h>

#include "callargument.h"

namespace llvm {
    class Module;
    class Type;
    class Value;
    class Function;
    class DataLayout;
};

class EvalType;

class OperatorEngine
{
public:
    OperatorEngine();
    void init();

public:
    // 类似std::bind
    QString bind(llvm::Module *mod, QString symbol, QString klass,
                 QVector<QVariant> uargs, QVector<QVariant> dargs,
                 QVector<MetaTypeVariant> mtdargs,
                 bool is_static, void *kthis);
    // 针对有些需要返回record类对象的方法，却返回了i32，这时需要做一个后处理。see issue #2。
    void elem_or_record_post_return();
    
    int getClassAllocSize(llvm::Module *mod, QString klass);
    llvm::DataLayout *getDataLayout(llvm::Module *mod);
    
private:
    // 正确地从 mod 和 mtmod两者中选择合适的类型
    llvm::Type *uniqTy(llvm::Module *mod, QString tystr);
    std::vector<llvm::Value*>
    ConvertToCallArgs(llvm::Module *module, llvm::IRBuilder<> &builder,
                      QVector<QVariant> uargs, QVector<QVariant> dargs,
                      QVector<MetaTypeVariant> mtdargs,
                      llvm::Function *dstfun, bool has_this);
    bool instcpy();
    // 默认参数临时值生成指令拷贝
    // 假设已经有InsertPoint
    QHash<QString, llvm::Value*>
    darg_instcpy(llvm::Module *mod, llvm::IRBuilder<> &builder);
};

#endif /* OPERATORENGINE_H */

















