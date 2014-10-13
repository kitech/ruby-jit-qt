
#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include <clang/CodeGen/ModuleBuilder.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Tool.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/ExternalASTSource.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>

#include <llvm/Support/Host.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#include "compilerengine.h"

void test_raw_codegen()
{
    clang::CompilerInstance ci;
    ci.createASTContext();
    ci.createDiagnostics();
    
    clang::ASTContext &astctx = ci.getASTContext();
    clang::DiagnosticsEngine &diag = ci.getDiagnostics();
    llvm::LLVMContext &vmctx = llvm::getGlobalContext();
    llvm::Module mod("piecegen", vmctx);
    clang::CodeGenOptions &cgopt = ci.getCodeGenOpts();

    llvm::DataLayout dlo("hehhe");
    clang::CodeGen::CodeGenModule cgmod(astctx, cgopt, mod, dlo, diag);

    cgmod.EmitAnnotationString("hhhh");

    clang::CodeGen::CodeGenFunction cgf(cgmod);
}


CompilerEngine::CompilerEngine()
{
    this->initCompiler();
}

CompilerEngine::~CompilerEngine()
{
}

bool CompilerEngine::initCompiler()
{
    clang::CompilerInstance *cis  = new clang::CompilerInstance();
    clang::CompilerInvocation *civ = new clang::CompilerInvocation();

    cis->createDiagnostics();
    cis->createFileManager();
    cis->createSourceManager(cis->getFileManager());
    cis->createFrontendTimer();
    // cis.createASTContext();
    // cis.createModuleManager();

    std::string path = "./myjitqt";
    clang::driver::Driver *drv = new clang::driver::Driver(path, llvm::sys::getProcessTriple(),
                                                          cis->getDiagnostics());
    drv->setTitle("myjitclangpp");
    drv->setCheckInputsExist(false);

    this->mcis = cis;
    this->mciv = civ;
    this->mdrv = drv;

    static char *argv[] = {
        (char*)"myjitqtrunner", (char*)"flycode.cxx",
        (char*)"-fPIC", (char*)"-x", (char*)"c++", 
        (char*)"-I/usr/include/qt", (char*)"-I/usr/include/qt/QtCore",
        (char*)"-I/usr/lib/clang/3.5.0/include",
    };
    static int argc = 8;
    char **targv = argv;

    llvm::SmallVector<const char*, 16> drv_args(targv, targv + argc);
    drv_args.push_back("-S");

    clang::driver::Compilation *C = drv->BuildCompilation(drv_args);

    const clang::driver::JobList &Jobs = C->getJobs();
    const clang::driver::Command *Cmd = llvm::cast<clang::driver::Command>(*Jobs.begin());
    const clang::driver::ArgStringList &CCArgs = Cmd->getArguments();

    // clang::CompilerInvocation *civ = new clang::CompilerInvocation();
    bool bret = clang::CompilerInvocation::CreateFromArgs(*civ,
                                                          const_cast<const char**>(CCArgs.data()),
                                                          const_cast<const char**>(CCArgs.data()) + CCArgs.size(),
                                                          cis->getDiagnostics());

    cis->setInvocation(civ);

    return true;
}

bool CompilerEngine::tryCompile(clang::CXXRecordDecl *decl, clang::ASTContext &ctx)
{
    clang::DiagnosticsEngine &diag = this->mcis->getDiagnostics();
    llvm::LLVMContext &vmctx = llvm::getGlobalContext();
    clang::CodeGenOptions &cgopt = this->mcis->getCodeGenOpts();
    clang::TargetOptions &tgopt = this->mcis->getTargetOpts();

    clang::ExternalASTSource *eas = ctx.getExternalSource();

    clang::CodeGenerator *cgtor = clang::CreateLLVMCodeGen(diag, "testmm", cgopt, tgopt, vmctx);
    qDebug()<<cgtor;
    
    cgtor->HandleTranslationUnit(ctx);
    int cnter = 0;
    for (auto it = decl->method_begin(); it != decl->method_end(); it ++, cnter++) {
        clang::CXXMethodDecl *mthdecl = *it;
        if (cnter < 100 && QString(mthdecl->getName().data()) == "length" ) { 
            cgtor->HandleInlineMethodDefinition(mthdecl);
        }
    }    

    llvm::Module *mod = cgtor->GetModule();
    qDebug()<<mod;
    mod->dump();

    /*
      void EmitBackendOutput(DiagnosticsEngine &Diags, const CodeGenOptions &CGOpts,
                         const TargetOptions &TOpts, const LangOptions &LOpts,
                         StringRef TDesc, llvm::Module *M, BackendAction Action,
                         raw_ostream *OS);
    */

    

    
    return false;
}

bool CompilerEngine::tryCompile3(clang::CXXRecordDecl *decl, clang::ASTContext &ctx)
{
    bool bret;
    QDateTime btime = QDateTime::currentDateTime();

    this->mcis->setASTContext(&ctx); // 这个instance不能和这个ctx结合使用？
    // const char *pcode = strdup(code.toLatin1().data());
    const char *pcode = strdup("\n void abc() { QString(\"abcefg\").length(); } ");
    llvm::MemoryBuffer *mbuf = llvm::MemoryBuffer::getMemBuffer(pcode);
    clang::PreprocessorOptions &ppOpt = this->mcis->getPreprocessorOpts();
    ppOpt.addRemappedFile("flycode.cxx", mbuf);

    clang::EmitLLVMOnlyAction llvm_only_action;
    bret = mcis->ExecuteAction(llvm_only_action);
    llvm::Module *mod = llvm_only_action.takeModule();
    QDateTime etime = QDateTime::currentDateTime();
    qDebug()<<"compile code done."<<bret<<mod<<btime.msecsTo(etime);

    mod->dump();

    return false;
}

bool CompilerEngine::tryCompile2(clang::CXXRecordDecl *decl, clang::ASTContext &ctx)
{
    qDebug()<<"heeeeee...";
    clang::CompilerInstance ci;
    // ci.createASTContext();
    ci.createDiagnostics();
    ci.createFileManager();

    qDebug()<<"heeeeee...";    
    clang::ASTContext &astctx = ci.getASTContext();
    clang::DiagnosticsEngine &diag = ci.getDiagnostics();
    llvm::LLVMContext &vmctx = llvm::getGlobalContext();
    llvm::Module mod("piecegen", vmctx);
    llvm::Module libmod("libcodes", vmctx);
    clang::CodeGenOptions &cgopt = ci.getCodeGenOpts();
    qDebug()<<"heeeeee...";
    llvm::DataLayout dlo("e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128");
    qDebug()<<"heeeeee...";
    clang::CodeGen::CodeGenModule cgmod(ctx, cgopt, mod, dlo, diag);

    // cgmod.EmitAnnotationString("hhhh");

    qDebug()<<"heeeeee...";
    clang::CodeGen::CodeGenFunction cgf(cgmod, true);

    qDebug()<<"heeeeee...";
    // cgmod.EmitTopLevelDecl(decl);
    // cgmod.AddDependentLib("tlib");
    // cgmod.getSize(5);
    // cgmod.getMangledName(decl);
    // cgmod.addDeferredVTable(decl);
    // cgmod.emitLLVMUsed();
    // cgmod.GetAddrOfGlobal(clang::cast<clang::CXXRecordDecl*>(decl));
    cgmod.GetAddrOfConstantCString("cccccccc66666"); // OK expr
    int cnter = 0;
    for (auto it = decl->method_begin(); it != decl->method_end(); it ++, cnter++) {
        clang::CXXMethodDecl *mthdecl = *it;
        if (cnter < 100 && QString(mthdecl->getName().data()) == "length" ) { 
            // cgmod.GetAddrOfGlobal(mthdecl);
            // cgmod.addDeferredVTable(decl);
            // cgmod.addUsedGlobal(mod.getNamedValue(cgmod.getMangledName(mthdecl)));
            // cgmod.addCompilerUsedGlobal(mod.getNamedValue(cgmod.getMangledName(mthdecl)));
            // cgmod.EmitGlobal(mthdecl);
            llvm::Constant * v = cgmod.GetAddrOfFunction(mthdecl, 0, false, false);
            if (mthdecl->isInlined()) {
                mthdecl->dumpColor();
                mthdecl->getBody()->dumpColor();
                clang::CodeGen::FunctionArgList fal;
                clang::QualType retype = mthdecl->getReturnType();
                llvm::Function *f = llvm::cast<llvm::Function>(v);
                
                
                // cgf.EmitFunctionBody(fal, mthdecl->getBody());
                // cgmod.addUsedGlobal(v);
                // llvm::Function *f = llvm::cast<llvm::Function>(v);
                // llvm::BasicBlock *blk = cgf.createBasicBlock("nnc", f);
                // llvm::BasicBlock *blk = llvm::BasicBlock::Create(vmctx, "nn", f);
                // cgf.EmitBranch(blk);
                // cgf.EmitBlockAfterUses(blk);

                // qDebug()<<clang::isa<llvm::Function>(v)<<v->getName().data()
                //         <<"hav body:"<<mthdecl->hasBody()
                //         <<"have ip1:"<<cgf.HaveInsertPoint();

                // mod.SetInsertPoint(blk);
                // qDebug()<<blk;
                // cgf.EmitBlock(blk);
                // cgf.EnsureInsertPoint();
                    // cgf.EmitCompoundStmt(*llvm::cast<clang::CompoundStmt>(mthdecl->getBody()));
                // f->addFnAttr(llvm::Attribute::AlwaysInline);
                // cgmod.EmitGlobalFunctionDefinition(mthdecl);
                // cgmod.EmitGlobalDefinition(mthdecl);
                // cgmod.setFunctionLinkage(mthdecl, f);

                if (mthdecl->hasBody()) {
                    clang::Stmt *stmt = mthdecl->getBody();
                    // stmt->dump();
                    // qDebug()<<stmt->getStmtClassName()<<"have ip:"<<cgf.HaveInsertPoint();

                    // cgf.EmitCompoundStmt(*llvm::cast<clang::CompoundStmt>(stmt));
                    // mod.dump();

                }
            }
            qDebug()<<mthdecl->getName().data()<<mthdecl->isInlined()<<mthdecl->hasBody();
        }

    }


    qDebug()<<"method count:"<<cnter;
    // decl->dump();
    

    qDebug()<<"heeeeee...";
    mod.dump();
    qDebug()<<"heeeeee...======";
    
    return false;
}

