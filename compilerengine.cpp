
#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include <clang/CodeGen/ModuleBuilder.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/CodeGen/BackendUtil.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Tool.h>
#include <clang/Parse/ParseAST.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/ExternalASTSource.h>
#include <clang/AST/Type.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/ASTUnit.h>

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

bool CompilerEngine::tryCompile(clang::CXXRecordDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit)
{
    clang::DiagnosticsEngine &diag = this->mcis->getDiagnostics();
    llvm::LLVMContext &vmctx = llvm::getGlobalContext();
    clang::CodeGenOptions &cgopt = this->mcis->getCodeGenOpts();
    clang::TargetOptions &tgopt = this->mcis->getTargetOpts();
    const clang::LangOptions &lgopt = ctx.getLangOpts();

    clang::ExternalASTSource *eas = ctx.getExternalSource();

    clang::CodeGenerator *cgtor = clang::CreateLLVMCodeGen(diag, "testmm", cgopt, tgopt, vmctx);
    qDebug()<<cgtor<<eas;

    
    cgtor->Initialize(ctx);
    cgtor->HandleTranslationUnit(ctx);
    cgtor->HandleVTable(decl, true);
    cgtor->HandleTagDeclDefinition(decl);

    int cnter = 0;
    for (auto it = decl->method_begin(); it != decl->method_end(); it ++, cnter++) {
        clang::CXXMethodDecl *mthdecl = *it;
        if (cnter < 1000 /*&& QString(mthdecl->getName().data()) == "length" */) { 
            cgtor->HandleInterestingDecl(clang::DeclGroupRef(mthdecl));
            cgtor->HandleTopLevelDecl(clang::DeclGroupRef(mthdecl));
            cgtor->HandleInlineMethodDefinition(mthdecl);
            cgtor->HandleCXXImplicitFunctionInstantiation(mthdecl);
        }
    }    

    qDebug()<<"mth cnter:"<<cnter;
    qDebug()<<"=========";
    cgtor->PrintStats();
    qDebug()<<"=========";
    llvm::Module *mod = cgtor->GetModule();
    qDebug()<<mod;
    mod->dump();

    std::string stmc;
    llvm::raw_string_ostream stmo(stmc);
    clang::EmitBackendOutput(diag, cgopt, tgopt, lgopt, "thisok", mod, clang::Backend_EmitLL, &stmo);
    /*
      void EmitBackendOutput(DiagnosticsEngine &Diags, const CodeGenOptions &CGOpts,
                         const TargetOptions &TOpts, const LangOptions &LOpts,
                         StringRef TDesc, llvm::Module *M, BackendAction Action,
                         raw_ostream *OS);
    */
    qDebug()<<stmo.str().length();// <<stmo.str().c_str();
    
    return false;
}

bool CompilerEngine::tryCompile3(clang::CXXRecordDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit)
{
    bool bret;
    QDateTime btime = QDateTime::currentDateTime();

    this->mcis->setASTContext(&ctx); // 这个instance不能和这个ctx结合使用？
    // this->mcis->setSema(&unit->getSema());
    // const char *pcode = strdup(code.toLatin1().data());
    const char *pcode = strdup("\n void abc() { QString(\"abcefg\").length(); } ");
    llvm::MemoryBuffer *mbuf = llvm::MemoryBuffer::getMemBuffer(pcode);
    clang::PreprocessorOptions &ppOpt = this->mcis->getPreprocessorOpts();
    ppOpt.addRemappedFile("flycode.cxx", mbuf);
    // 解析这段代码，结合已有的astfile查找标识符号。???

    clang::ParseAST(unit->getSema());

    clang::EmitLLVMOnlyAction llvm_only_action;
    // clang::FrontendInputFile fif("./data/qthdrsrc.ast", clang::IK_AST);
    // llvm_only_action.setCurrentInput(fif, unit); // 不管用，不知道为什么。
    qDebug()<<"ast support:"<<llvm_only_action.hasASTFileSupport();
    qDebug()<<"what ctx:"<<&this->mcis->getASTContext()<<&ctx;

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
    clang::CodeGen::CodeGenFunction cgf(cgmod, false);

    qDebug()<<"heeeeee...";
    // cgmod.EmitTopLevelDecl(decl);
    // cgmod.AddDependentLib("tlib");
    // cgmod.getSize(5);
    // cgmod.getMangledName(decl);
    // cgmod.addDeferredVTable(decl);
    // cgmod.emitLLVMUsed();
    // cgmod.GetAddrOfGlobal(clang::cast<clang::CXXRecordDecl*>(decl));
    // cgmod.GetAddrOfConstantCString("cccccccc66666"); // OK expr
    // cgmod.EmitNullConstantForBase(decl);

    if (1) {
        int cnter = 0;
        for (auto it = decl->method_begin(); it != decl->method_end(); it ++, cnter++) {
            clang::CXXMethodDecl *mthdecl = *it;
            if (!clang::isa<clang::CXXConstructorDecl>(mthdecl)) continue;
            if (!mthdecl->isInlined()) continue;

            clang::CXXConstructorDecl *ctor = clang::cast<clang::CXXConstructorDecl>(mthdecl);
            clang::CodeGen::CodeGenTypes &cgtypes = cgmod.getTypes();

            if (ctor->isCopyConstructor()) continue;

            int cnter2 = 0;
            for (auto it = ctor->init_begin(); it != ctor->init_end(); it ++, cnter2 ++) {
                qDebug()<<"init..."<<*it;
            }
            // if (cnter2 != 0) continue;   // 如果没有CtorInitializer

            int cnter3 = 0;
            for (auto it = ctor->redecls_begin(); it != ctor->redecls_end(); it ++, cnter3++) {
                qDebug()<<"redecl:"<<*it;
                (*it)->dumpColor();
            }

            if (cnter3 == 2) {
                qDebug()<<"rewrite to redecl decl...";
                mthdecl = clang::cast<clang::CXXMethodDecl>(*(++ctor->redecls_begin()));
                ctor = clang::cast<clang::CXXConstructorDecl>(mthdecl);
            }
            // mthdecl->hasInlineBody()方法起到判断的决定作用，这样才能用definition的decl*生成正确的ll代码。
            qDebug()<<"init count:"<<cnter2<<mthdecl->isThisDeclarationADefinition()
                    <<mthdecl->isOutOfLine()<<mthdecl->isDefinedOutsideFunctionOrMethod()
                    <<"redecl count:"<<cnter3
                    <<",inline body:"<<mthdecl->hasInlineBody()
                    <<",is first:"<<mthdecl->isFirstDecl();

            mthdecl->getBody()->dumpColor();
            mthdecl->dumpColor();
            // mthdecl->getCanonicalDecl()->dumpColor();
            // mthdecl->getCorrespondingMethodInClass(decl)->dumpColor();


            // try ctor base , 不能生成正确的Base代码
            const clang::CodeGen::CGFunctionInfo &FIB = 
                cgtypes.arrangeCXXConstructorDeclaration(ctor, clang::Ctor_Base);
            llvm::FunctionType *FTyB = cgtypes.GetFunctionType(FIB);
            llvm::Constant *ctor_base_val = 
                // cgm.GetAddrOfFunction(clang::GlobalDecl(ctor, clang::Ctor_Base), FTyB, false, false);
                cgmod.GetAddrOfCXXConstructor(ctor, clang::Ctor_Base, &FIB, true);
            llvm::Function *ctor_base_fn = clang::cast<llvm::Function>(ctor_base_val);
            clang::CodeGen::FunctionArgList alist;


            cgmod.setFunctionLinkage(clang::GlobalDecl(ctor, clang::Ctor_Base), ctor_base_fn);
            cgf.GenerateCode(clang::GlobalDecl(ctor, clang::Ctor_Base), ctor_base_fn, FIB);
            // cgf.EmitCtorPrologue(ctor, clang::Ctor_Base, alist);
            // clang::CodeGen::CodeGenFunction(cgm).EmitCtorPrologue(ctor, clang::Ctor_Base, alist);
            // clang::CodeGen::CodeGenFunction(cgm).GenerateCode(clang::GlobalDecl(ctor, clang::Ctor_Base), ctor_base_fn, FIB);
            // cgmod.EmitGlobal(clang::GlobalDecl(ctor, clang::Ctor_Base));
            // cgf.FinishFunction();

            // mod.dump();
            // break;
            qDebug()<<"========================";
        }
        mod.dump();
        return false;
    }
    int cnter = 0;
    for (auto it = decl->method_begin(); it != decl->method_end(); it ++, cnter++) {
        clang::CXXMethodDecl *mthdecl = *it;
        QString symname = QString(cgmod.getMangledName(mthdecl).data());
        if (symname.indexOf("QStringC") >= 0) {
            clang::CXXConstructorDecl *ctor = clang::cast<clang::CXXConstructorDecl>(mthdecl);
            qDebug()<<cgmod.getMangledName(mthdecl).data();
            qDebug()<<cgmod.getMangledName(clang::GlobalDecl(ctor, clang::Ctor_Base)).data();
            qDebug()<<cgmod.getMangledName(clang::GlobalDecl(ctor, clang::Ctor_CompleteAllocating)).data();
            for (auto it = ctor->init_begin(); it != ctor->init_end(); it ++) {
                clang::CXXCtorInitializer *cti = *it;
                clang::Expr *ctie = cti->getInit();
                qDebug()<<cti;
                ctor->dumpColor();
                ctie->dumpColor();
            }
        }
        cgmod.EmitTopLevelDecl(mthdecl);
        cgmod.EmitGlobal(mthdecl);
        if (cnter < 1000 && QString(mthdecl->getName().data()) == "") { 
            if (mthdecl->isInlined() && clang::isa<clang::CXXConstructorDecl>(mthdecl)
                ) {
                qDebug()<<mthdecl->hasBody()<<mthdecl->hasTrivialBody()
                        <<mthdecl->isThisDeclarationADefinition()
                        <<mthdecl->isOutOfLine()
                        <<mthdecl->hasInlineBody()
                        <<decl->hasInClassInitializer();
                // mthdecl->dumpColor();
                // mthdecl->getBody()->dumpColor();

                auto genmth = [](clang::CodeGen::CodeGenModule &cgm,
                                 clang::CodeGen::CodeGenFunction &cgf,
                                 clang::CXXMethodDecl *decl) -> bool {
                    clang::CXXConstructorDecl *ctor = clang::cast<clang::CXXConstructorDecl>(decl);
                    clang::CodeGen::CodeGenTypes &cgtypes = cgm.getTypes();

                    const clang::CodeGen::CGFunctionInfo &FI = 
                    cgtypes.arrangeCXXConstructorDeclaration(ctor, clang::Ctor_Complete);
                    
                    llvm::FunctionType *FTy = cgtypes.GetFunctionType(FI);

                    llvm::Constant * v = cgm.GetAddrOfFunction(ctor,
                                                               FTy, false, false);

                    llvm::Function *f = llvm::cast<llvm::Function>(v);
                    qDebug()<<"dbg func:"<<f->arg_size()
                    << cgf.CapturedStmtInfo;
                    // f->viewCFG();
                    clang::Stmt *stmt = decl->getBody();
                    clang::CodeGen::FunctionArgList alist;
                    cgf.GenerateCode(decl, f, FI);
                    f->addFnAttr(llvm::Attribute::AlwaysInline);
                    cgm.setFunctionLinkage(decl, f);

                    // cgm.EmitCXXConstructor(ctor, clang::Ctor_Base);
                    // try ctor base , 不能生成正确的Base代码
                    const clang::CodeGen::CGFunctionInfo &FIB = 
                    cgtypes.arrangeCXXConstructorDeclaration(ctor, clang::Ctor_Base);
                    llvm::FunctionType *FTyB = cgtypes.GetFunctionType(FIB);
                    llvm::Constant *ctor_base_val = 
                    // cgm.GetAddrOfFunction(clang::GlobalDecl(ctor, clang::Ctor_Base), FTyB, false, false);
                    cgm.GetAddrOfCXXConstructor(ctor, clang::Ctor_Base, &FIB, true);
                    llvm::Function *ctor_base_fn = clang::cast<llvm::Function>(ctor_base_val);
                    // cgf.EmitCtorPrologue(ctor, clang::Ctor_Base, alist);
                    cgm.setFunctionLinkage(clang::GlobalDecl(ctor, clang::Ctor_Base), ctor_base_fn);
                    // clang::CodeGen::CodeGenFunction(cgm).EmitCtorPrologue(ctor, clang::Ctor_Base, alist);
                    // clang::CodeGen::CodeGenFunction(cgm).GenerateCode(clang::GlobalDecl(ctor, clang::Ctor_Base), ctor_base_fn, FIB);
                    // cgm.EmitGlobal(clang::GlobalDecl(ctor, clang::Ctor_Base));
                    
                    return false;
                };
                QDateTime btime = QDateTime::currentDateTime();
                genmth(cgmod, cgf, mthdecl);
                QDateTime etime = QDateTime::currentDateTime();
                qDebug()<<"gen func time:"<<btime.msecsTo(etime);
                mod.dump();

                // break;
            }
        }
        if (0 && cnter < 100 && QString(mthdecl->getName().data()) == "length" ) { 
            // cgmod.GetAddrOfGlobal(mthdecl);
            // cgmod.addDeferredVTable(decl);
            // cgmod.addUsedGlobal(mod.getNamedValue(cgmod.getMangledName(mthdecl)));
            // cgmod.addCompilerUsedGlobal(mod.getNamedValue(cgmod.getMangledName(mthdecl)));
            // cgmod.EmitGlobal(mthdecl);

            if (mthdecl->isInlined()) {
                mthdecl->dumpColor();
                mthdecl->getBody()->dumpColor();

                auto genmth = [](clang::CodeGen::CodeGenModule &cgm,
                                 clang::CodeGen::CodeGenFunction &cgf,
                                 clang::CXXMethodDecl *decl) -> bool {
                    clang::CodeGen::CodeGenTypes &cgtypes = cgm.getTypes();

                    const clang::CodeGen::CGFunctionInfo &FI = 
                    cgtypes.arrangeCXXMethodDeclaration(decl);
                    
                    llvm::FunctionType *FTy = cgtypes.GetFunctionType(FI);

                    clang::QualType retype = decl->getReturnType();
                    llvm::Type *lvtype = cgtypes.ConvertType(retype);
                    llvm::Constant * v = cgm.GetAddrOfFunction(decl, FTy,
                                                               false, false);

                    llvm::Function *f = llvm::cast<llvm::Function>(v);
                    qDebug()<<"dbg func:"<<f->arg_size()
                    << cgf.CapturedStmtInfo;
                    // f->viewCFG();
                    clang::Stmt *stmt = decl->getBody();
                    clang::CodeGen::FunctionArgList alist;
                    cgf.GenerateCode(decl, f, FI);
                    f->addFnAttr(llvm::Attribute::AlwaysInline);
                    cgm.setFunctionLinkage(decl, f);

                    return false;
                };
                QDateTime btime = QDateTime::currentDateTime();
                genmth(cgmod, cgf, mthdecl);
                QDateTime etime = QDateTime::currentDateTime();
                qDebug()<<"gen func time:"<<btime.msecsTo(etime);

                // f->addFnAttr(llvm::Attribute::AlwaysInline);
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

