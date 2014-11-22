#include "fix_clang_undef_eai.h"
#include <QtCore>

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/ModuleLoader.h"

////////

////////
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include <llvm/AsmParser/Parser.h>
#include "llvm/Support/Host.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/IR/ValueSymbolTable.h"



static void debug_module(llvm::Module *mod)
{
    qDebug()<<"identifier:"<<mod->getModuleIdentifier().c_str();
    qDebug()<<"target triple:"<<mod->getTargetTriple().c_str();
    qDebug()<<"inline asm:" << mod->getModuleInlineAsm().c_str();

    llvm::GlobalValue * mgv = mod->getNamedValue("_ZL1s");
    qDebug()<<"mgv:s:"<<mgv;
    // mod->dump();
    qDebug()<<"data layout str:"<<mod->getDataLayoutStr().c_str();

    llvm::Function *vfun = mod->getFunction("main");
    qDebug()<<"main func:"<<vfun;

    llvm::Function *etyfn = NULL;
    QString mangle_name;
    llvm::ValueSymbolTable &vst = mod->getValueSymbolTable();
    for (auto sit = vst.begin(); sit != vst.end(); sit++) {
        auto k = sit->first();
        auto v = sit->second;
        qDebug()<<""<<k.data()<<v->getName().data();
        //  v->dump();
    }

    auto q_string_t = mod->getTypeByName("class.QString");
    q_string_t->dump();

    int i = 0;
    for (auto it = mod->global_begin(); it != mod->global_end(); it ++) {
        qDebug()<<"global:"<<i;
        i++;
    }
    qDebug()<<"global cc:"<<i<<mod->global_empty();

    i = 0;
    for (auto it = mod->alias_begin(); it != mod->alias_end(); it ++) {
        i++;        
    }
    qDebug()<<"global cc:"<<i;
}

void load_types_module ()
{
    QFile fp("./qttypes.ll");
    fp.open(QIODevice::ReadOnly);
    QByteArray asm_data = fp.readAll();
    fp.close();

    const char *asm_str = strdup(asm_data.data());
    qDebug()<<"llstrlen:"<<strlen(asm_str);

    llvm::LLVMContext &gctx = llvm::getGlobalContext();
    llvm::SMDiagnostic smdiag;
    llvm::Module *rmod = NULL;

    rmod = llvm::ParseAssemblyString(asm_str, NULL, smdiag, gctx);
    qDebug()<<rmod;
    // rmod->dump();
    qDebug()<<"========";

    debug_module(rmod);
}

void call_op_new(llvm::IRBuilder<> &builder, llvm::Module *mod)
{
    llvm::Type* TQString = mod->getTypeByName("class.QString");
    void *mem = calloc(1, 16);

    std::vector<llvm::Type*> putsArgs;
    putsArgs.push_back(TQString->getPointerTo());
    llvm::ArrayRef<llvm::Type*> argsRef(putsArgs);
    llvm::FunctionType *ctor_func_rtype = llvm::FunctionType::get(builder.getVoidTy(), 
                                                                  argsRef,false);
    llvm::Constant *ctor_fun = mod->getOrInsertFunction("_ZN7QStringC2Ev", ctor_func_rtype);

    llvm::Value *helloWorld = builder.CreateGlobalStringPtr("hello world!\n");
    llvm::Value *sval = builder.CreateAlloca(TQString->getPointerTo());
    builder.CreateCall(ctor_fun, sval);
    // === new QString???
}

void run_code_jit()
{
    llvm::LLVMContext &ctx = llvm::getGlobalContext();
    llvm::Module *module = new llvm::Module("top", ctx);
    llvm::IRBuilder<> builder(ctx);

    llvm::FunctionType *funcType = llvm::FunctionType::get(builder.getInt32Ty(), false);
    // llvm::FunctionType *funcType = llvm::FunctionType::get(builder.getVoidTy(), false);
    llvm::Function *mainFunc = 
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);

    llvm::BasicBlock *entry = llvm::BasicBlock::Create(ctx, "entrypoint", mainFunc);
    builder.SetInsertPoint(entry);

    llvm::Value *helloWorld = builder.CreateGlobalStringPtr("hello world!\n");

    std::vector<llvm::Type*> putsArgs;
    putsArgs.push_back(builder.getInt8Ty()->getPointerTo());
    llvm::ArrayRef<llvm::Type*> argsRef(putsArgs);

    llvm::FunctionType *putsType = llvm::FunctionType::get(builder.getInt32Ty(), argsRef, false);
    llvm::Constant *putsFunc = module->getOrInsertFunction("puts", putsType);

    builder.CreateCall(putsFunc, helloWorld);
    // builder.CreateRetVoid();
    // ContaintInt * is a Value *
    // 
    // 
    llvm::Value *retStatus = builder.getInt32(123);
    builder.CreateRet(retStatus);

    putsType->dump();
    llvm::Type *q_string_t = module->getTypeByName("class.QVariant");
    qDebug()<<"=======";
    q_string_t->dump();
    qDebug()<<"=======";

    llvm::FunctionType *qft = llvm::FunctionType::get(q_string_t, false);
    llvm::Constant *qfun = module->getOrInsertFunction("qfunxx", qft);

    call_op_new(builder, module);

    module->dump();

    qDebug()<<"jit done.";
}


int main(int argc, char **argv)
{
    load_types_module();
    run_code_jit();
    return 0;
}
