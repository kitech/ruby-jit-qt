
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

#include "operatorengine.h"

QString OperatorEngine::bind(llvm::Module *mod, QString symbol, QVector<QVariant> uargs)
{
    QString lamsym;
    
    llvm::LLVMContext &vmctx = mod->getContext();
    llvm::IRBuilder<> builder(vmctx);
    llvm::Function *dstfun = mod->getFunction(symbol.toLatin1().data());
    llvm::Function *lamfun = NULL;
    const char *lamname = "jit_main"; // TODO, rt mangle
    
    auto c_lamfun = mod->getOrInsertFunction(lamname, dstfun->getReturnType(), NULL);
    lamfun = (decltype(lamfun))(c_lamfun); // cast it
    
    
    mod->dump();

    return lamsym;
}
