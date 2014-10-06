#ifndef CLVM_OPERATOR_H
#define CLVM_OPERATOR_H

#include <QtCore>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/TypeBuilder.h"


bool irop_new(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder, 
            llvm::Module *module, QString klass);

bool irop_call(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder, 
               llvm::Module *module, void *kthis, QString klass, QString method);

class IROperator
{
public:
    IROperator();
    ~IROperator();

    bool init();

public:
    bool knew(QString klass); // klass new
    bool kdelete(void *kthis, QString klass);
    bool call(void *kthis, QString klass, QString method, QVector<QVariant> args,
              QString symbol_name);

public:
    QString resolve_return_type(QString klass, QString method,
                                QVector<QVariant> args, QString mangle_name);

public:
    llvm::Module *tmod = NULL; // for jit types
    llvm::Module *dmod = NULL; // default irgen module
    llvm::LLVMContext &ctx;
    llvm::IRBuilder<> builder;
};


#endif /* CLVM_OPERATOR_H */










