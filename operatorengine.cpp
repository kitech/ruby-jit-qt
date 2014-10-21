
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/Support/SourceMgr.h>

#include "operatorengine.h"

OperatorEngine::OperatorEngine()
{
    this->init();
}

void OperatorEngine::init()
{
    auto load_jit_types_module = []() -> llvm::Module* {
        llvm::LLVMContext *ctx = new llvm::LLVMContext();
        llvm::Module *module = new llvm::Module("jit_types_in_oe", *ctx);

        // load module
        QFile fp("./metalize/jit_types.ll");
        fp.open(QIODevice::ReadOnly);
        QByteArray asm_data = fp.readAll();
        fp.close();

        char *asm_str = strdup(asm_data.data());
        qDebug()<<"llstrlen:"<<strlen(asm_str);

        llvm::SMDiagnostic smdiag;
        llvm::Module *rmod = NULL;

        rmod = llvm::ParseAssemblyString(asm_str, NULL, smdiag, *ctx);
        qDebug()<<"rmod="<<rmod;
        free(asm_str);

        return rmod;
    };
    this->mtmod = load_jit_types_module();
}

#include "invokestorage.h"
InvokeStorage gis2;

QString OperatorEngine::bind(llvm::Module *mod, QString symbol, void *kthis, QVector<QVariant> uargs
                             , QVector<QVariant> dargs)
{
    QString lamsym;
    
    llvm::LLVMContext &vmctx = mod->getContext();
    llvm::IRBuilder<> builder(vmctx);
    llvm::Function *dstfun = mod->getFunction(symbol.toLatin1().data());
    llvm::Function *lamfun = NULL;
    const char *lamname = "jit_main"; // TODO, rt mangle
    
    auto c_lamfun = mod->getOrInsertFunction(lamname, dstfun->getReturnType(), NULL);
    lamfun = (decltype(lamfun))(c_lamfun); // cast it

    std::vector<llvm::Value*> callee_arg_values;
    QString klass = "QString";
    llvm::Type *thisTy = this->mtmod->getTypeByName(QString("class.%1").arg(klass).toStdString());
    callee_arg_values.push_back(llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)kthis));

    builder.SetInsertPoint(llvm::BasicBlock::Create(mod->getContext(), "eee", lamfun));
    llvm::ArrayRef<llvm::Value*> callee_arg_values_ref(callee_arg_values);
    llvm::CallInst *cval = builder.CreateCall(dstfun, callee_arg_values_ref);

    qDebug()<<"sret:"<<dstfun->hasStructRetAttr()<<"noret:"<<dstfun->doesNotReturn();
    if (dstfun->hasStructRetAttr()) {
        
    } else if (dstfun->doesNotReturn() || dstfun->getReturnType() == builder.getVoidTy()) {
        builder.CreateRetVoid();
    } else {
        builder.CreateRet(cval);
    }

    mod->dump();

    lamsym = QString(lamname);
    return lamsym;
}
