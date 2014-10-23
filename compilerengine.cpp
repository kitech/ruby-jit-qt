
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
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/AST.h>
#include <clang/AST/ASTImporter.h>
#include <clang/AST/ExternalASTSource.h>
#include <clang/AST/Type.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/ASTUnit.h>

#include <llvm/Support/Host.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/AsmParser/Parser.h>


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

    // static llvm::Module *jit_types_mod = NULL;
    auto load_jit_types_module = []() -> llvm::Module* {
        llvm::LLVMContext *ctx = new llvm::LLVMContext();
        llvm::Module *module = new llvm::Module("jit_types_in_ce", *ctx);

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
    mtmod = load_jit_types_module();
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

// 第一种方式，拿到还未定义的symbol，到ast中查找
// 第二种方式，解析C++方法的源代码，找到这个未定义的symbol，从decl再把它编译成ll，效率更好。
void CompilerEngine::decl2def(llvm::Module *mod, clang::ASTContext &ctx,
                              clang::CodeGen::CodeGenModule &cgmod, 
                              clang::Decl *decl, int level, QHash<QString, bool> noinlined)
{
    llvm::Module *jit_types_mod = NULL;

    QHash<QString, bool> efuns;
    if (jit_types_mod == NULL) {
        jit_types_mod = mtmod;
        // jit_types_mod->dump();

        for (auto &f: jit_types_mod->getFunctionList()) {
            efuns.insert(QString(f.getName().data()), true);
        }
    }
    qDebug()<<"exists funcs:"<<efuns.count()<<efuns;

    int copyed = 0;
    QVector<QString> gfuns;
    for (auto &f: mod->getFunctionList()) {
        QString fname = f.getName().data();
        qDebug()<<"name:"<<f.getName().data()<<f.size()<<"decl:"<<f.isDeclaration();
        if (!f.isDeclaration()) {
            continue;
        }
        if (noinlined.contains(fname)) {
            continue;
        }

        // 如果判断是不是inline的呢。

        if (efuns.contains(fname)) {
            // copy function from prepared module
            qDebug()<<"copying func:"<<fname;
            continue;

            // copy可能遇到需要递归的指定，如call，或者一些全局变量也需要同时处理。
            // 还是比较复杂的。
            auto srcf = jit_types_mod->getFunction(fname.toLatin1().data());
            auto dstf = mod->getFunction(fname.toLatin1().data());
            
            llvm::IRBuilder<> builder(mod->getContext());
            // llvm::BasicBlock *entry = llvm::BasicBlock::Create(mod->getContext(),
            //                                                    "clvm_func_entrypoint", dstf);
            // builder.SetInsertPoint(entry);
            // qDebug()<<entry<<dstf;
            for (auto &blk: *srcf) {
                qDebug()<<","<<blk.getName().data()<<",";
                llvm::BasicBlock *tb = llvm::BasicBlock::Create(mod->getContext(),
                                                                blk.getName(), dstf);
                builder.SetInsertPoint(tb);                
                for (auto &ins: blk) {
                    auto ni = ins.clone();
                    builder.Insert(ni);
                    qDebug()<<&blk<<&ins<<ins.getName().data()<<"opc:"<<ins.getOpcodeName()<<ins.getOpcode();
                    if (ins.getOpcode() == llvm::Instruction::Call) {
                        llvm::CallInst *ci = llvm::cast<llvm::CallInst>(ni);
                        qDebug()<<"call arg num:"<<ci->getNumArgOperands();
                        auto called_func = ci->getArgOperand(0);
                        qDebug()<<"afun?"<<called_func->getName().data()
                                <<llvm::isa<llvm::Function>(called_func);
                        
                    }
                }
            }
            copyed ++;
            continue;
        }
        gfuns.append(fname);
    }

    qDebug()<<"need gen funs:"<<gfuns.count()<<copyed;
    if (gfuns.count() == 0 && copyed == 0) return;
    for (auto fname: gfuns) {
        
    }

    if (level > 10) {
        qDebug()<<"maybe too much recursive level.";
        exit(-1);
    }
    decl2def(mod, ctx, cgmod, decl, ++level, noinlined);
}


// todo 还需要一个递归生成的过程，多次检查生成的结果是否有inline方法
llvm::Module* CompilerEngine::conv_ctor(clang::ASTContext &ctx, clang::CXXConstructorDecl *ctor)
{
    clang::CompilerInstance ci;
    // ci.createASTContext();
    ci.createDiagnostics();
    ci.createFileManager();

    clang::DiagnosticsEngine &diag = ci.getDiagnostics();
    clang::CodeGenOptions &cgopt = ci.getCodeGenOpts();

    auto vmctx = new llvm::LLVMContext();
    auto mod = new llvm::Module("piecegen", *vmctx);

    llvm::DataLayout dlo("e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128");
    clang::CodeGen::CodeGenModule cgmod(ctx, cgopt, *mod, dlo, diag);
    auto &cgtypes = cgmod.getTypes();
    auto cgf = new clang::CodeGen::CodeGenFunction(cgmod);

    // assert(islined and hasinlinedbody)
    auto get_decl_with_body = [](clang::CXXConstructorDecl *ctor) -> decltype(ctor) {
        if (ctor->hasInlineBody()) return ctor;
        int cnt = 0;
        for (auto rd: ctor->redecls()) cnt++;
        if (cnt == 1) return ctor;

        for (auto rd: ctor->redecls()) {
            if (rd == ctor) continue;
            return (decltype(ctor))rd;
        }
        return 0;
    };
    ctor = get_decl_with_body(ctor);
    
    // try ctor base , 能生成正确的Base代码了
    const clang::CodeGen::CGFunctionInfo &FIB = 
        cgtypes.arrangeCXXConstructorDeclaration(ctor, clang::Ctor_Base);
    llvm::FunctionType *FTyB = cgtypes.GetFunctionType(FIB);
    llvm::Constant *ctor_base_val = 
        cgmod.GetAddrOfCXXConstructor(ctor, clang::Ctor_Base, &FIB, true);
    llvm::Function *ctor_base_fn = clang::cast<llvm::Function>(ctor_base_val);
    clang::CodeGen::FunctionArgList alist;

    QHash<QString, bool> noinlined; // 不需要生成define的symbol，在decl2def中使用。
    if (ctor->isInlined()) {
        cgmod.setFunctionLinkage(clang::GlobalDecl(ctor, clang::Ctor_Base), ctor_base_fn);
        cgf->GenerateCode(clang::GlobalDecl(ctor, clang::Ctor_Base), ctor_base_fn, FIB);
    } else {
        noinlined[ctor_base_fn->getName().data()] = true;
    }

    mod->dump();

    // ??? 没用了吧
    QHash<QString, llvm::Function*> usyms;
    for (auto &f: mod->getFunctionList()) {
        qDebug()<<"name:"<<f.getName().data()<<f.size()<<f.isDeclaration();
        if (f.isDeclaration()) usyms.insert(QString(f.getName().data()), &f);
    }

    ctor->dumpColor();
    // 遍历body stmt
    auto s = ctor->getBody();
    s->dumpColor();

    
    // 遍历Ctor init
    for (auto i: ctor->inits()) {
        auto s = i->getInit();
        auto cs = clang::cast<clang::CallExpr>(s);
        s->dumpColor();
        auto d = cs->getCalleeDecl();
        d->dumpColor();

        auto sd = clang::cast<clang::CXXMethodDecl>(d);
        qDebug()<<"call name:"<<cgmod.getMangledName(clang::cast<clang::CXXMethodDecl>(d)).data();

        // this->conv_ctor(ctx, sd);
        QString tmp = cgmod.getMangledName(sd).data();
        if (usyms.contains(tmp)) {
             llvm::Function *f = usyms.value(tmp);
             // need def
             qDebug()<<"need def:"<<tmp<<f;

        }
        break;
    }

    mod->dump(); 

    decl2def(mod, ctx, cgmod, ctor, 0, noinlined);

    mod->dump(); 

    return mod;
}

// 这个方法支持普通方法，static也可以,非template
// 已知问题，不支持模板类中的静态方法。
llvm::Module* CompilerEngine::conv_method(clang::ASTContext &ctx, clang::CXXMethodDecl *mth)
{
    clang::CompilerInstance ci;
    // ci.createASTContext();
    ci.createDiagnostics();
    ci.createFileManager();

    clang::DiagnosticsEngine &diag = ci.getDiagnostics();
    clang::CodeGenOptions &cgopt = ci.getCodeGenOpts();

    auto vmctx = new llvm::LLVMContext();
    auto mod = new llvm::Module("piecegen2", *vmctx);

    llvm::DataLayout dlo("e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128");
    clang::CodeGen::CodeGenModule cgmod(ctx, cgopt, *mod, dlo, diag);
    auto &cgtypes = cgmod.getTypes();
    auto cgf = new clang::CodeGen::CodeGenFunction(cgmod);

    // assert(islined and hasinlinedbody)
    auto get_decl_with_body = [](clang::CXXMethodDecl *mth) -> decltype(mth) {
        if (mth->hasInlineBody()) return mth;
        int cnt = 0;
        for (auto rd: mth->redecls()) cnt++;
        if (cnt == 1) return mth;

        for (auto rd: mth->redecls()) {
            if (rd == mth) continue;
            return (decltype(mth))rd;
        }
        return 0;
    };
    mth = get_decl_with_body(mth);

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
        qDebug()<<"dbg func:"<<f<<f->arg_size()
        << cgf.CapturedStmtInfo;
        // f->viewCFG();
        // clang::Stmt *stmt = decl->getBody();
        clang::CodeGen::FunctionArgList alist;
        cgf.GenerateCode(clang::GlobalDecl(decl), f, FI);
        f->addFnAttr(llvm::Attribute::AlwaysInline);
        cgm.setFunctionLinkage(decl, f);

        return false;
    };

    // TODO 无法生成正确的函数参数信息，所有参数类型全部成了i64了
    // eg. declare void @_ZN7QObject7connectEPKS_PKcS1_S3_N2Qt14ConnectionTypeE
    //     (%"class.QMetaObject::Connection"* sret, i64, i64, i64, i64, i32) #0
    auto genmth_decl = [](clang::CodeGen::CodeGenModule &cgm,
                          clang::CodeGen::CodeGenFunction &cgf,
                          clang::CXXMethodDecl *decl,
                          llvm::Module *mtmod) -> bool {
        clang::CodeGen::CodeGenTypes &cgtypes = cgm.getTypes();

        const clang::CodeGen::CGFunctionInfo &FI = 
        cgtypes.arrangeCXXMethodDeclaration(decl);
                    
        llvm::FunctionType *FTy = cgtypes.GetFunctionType(FI);

        clang::QualType retype = decl->getReturnType();
        llvm::Type *lvtype = cgtypes.ConvertType(retype);
        cgm.EmitGlobal(decl);
        llvm::Constant * v = cgm.GetAddrOfFunction(decl, FTy,
                                                   false, false);

        llvm::Function *f = llvm::cast<llvm::Function>(v);
        // FTy->dump();
        qDebug()<<"dbg func:"<<f<<f->arg_size()
        << cgf.CapturedStmtInfo<<decl->param_size();

        int cnter = 0;
        for (auto &arg: f->getArgumentList()) {
            if (arg.hasStructRetAttr()) continue; // skip sret
            if (!decl->isStatic()) continue;     // skip this
            // qDebug()<<cnter<<cgtypes.ConvertType(decl->getParamDecl(cnter)->getType());
            arg.mutateType(cgtypes.ConvertType(decl->getParamDecl(cnter++)->getType()));
            // 该函数执行后，mod->dump()看不出来变化，但是在用程序检测的时候却改变了。
        }

        return false;
    };


    if (mth->isInlined()) {
        genmth(cgmod, *cgf, mth);
    } else {
        genmth_decl(cgmod, *cgf, mth, mtmod);
    }
    
    mod->dump();

    return mod;
}

QString CompilerEngine::mangle_ctor(clang::ASTContext &ctx, clang::CXXConstructorDecl *ctor)
{
    clang::CompilerInstance ci;
    // ci.createASTContext();
    ci.createDiagnostics();
    ci.createFileManager();

    clang::DiagnosticsEngine &diag = ci.getDiagnostics();
    clang::CodeGenOptions &cgopt = ci.getCodeGenOpts();

    auto vmctx = new llvm::LLVMContext();
    auto mod = new llvm::Module("piecegen", *vmctx);

    llvm::DataLayout dlo("e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128");
    clang::CodeGen::CodeGenModule cgmod(ctx, cgopt, *mod, dlo, diag);
    auto &cgtypes = cgmod.getTypes();
    auto cgf = new clang::CodeGen::CodeGenFunction(cgmod);

    auto  vm_symname = cgmod.getMangledName(clang::GlobalDecl(ctor, clang::Ctor_Base));
    QString symname = vm_symname.data();
    return symname;
}

QString CompilerEngine::mangle_method(clang::ASTContext &ctx, clang::CXXMethodDecl *mth)
{
    
    clang::CompilerInstance ci;
    // ci.createASTContext();
    ci.createDiagnostics();
    ci.createFileManager();

    clang::DiagnosticsEngine &diag = ci.getDiagnostics();
    clang::CodeGenOptions &cgopt = ci.getCodeGenOpts();

    auto vmctx = new llvm::LLVMContext();
    auto mod = new llvm::Module("piecegen", *vmctx);

    llvm::DataLayout dlo("e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128");
    clang::CodeGen::CodeGenModule cgmod(ctx, cgopt, *mod, dlo, diag);
    auto &cgtypes = cgmod.getTypes();
    auto cgf = new clang::CodeGen::CodeGenFunction(cgmod);

    auto  vm_symname = cgmod.getMangledName(clang::GlobalDecl(mth));
    QString symname = vm_symname.data();
    return symname;
    return QString();
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

// test compileinstance with prepared astcontext
bool CompilerEngine::tryCompile4(clang::CXXRecordDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit)
{
    bool bret;
    QDateTime btime = QDateTime::currentDateTime();

    clang::ASTContext *pctx = &ctx;
    clang::FileManager &pfm = unit->getFileManager();
    clang::SourceManager &srcman = unit->getSourceManager();
    clang::DiagnosticsEngine &diag = ctx.getDiagnostics();
    clang::Sema &sema = unit->getSema();
    clang::Preprocessor &pp = unit->getPreprocessor();
    const clang::TargetInfo &ti = ctx.getTargetInfo();

    clang::CompilerInvocation *civ = new clang::CompilerInvocation();
    clang::CompilerInstance *cis  = new clang::CompilerInstance();
    cis->createDiagnostics();

    std::string path = "./myjitqt";
    clang::driver::Driver *drv = new clang::driver::Driver(path, llvm::sys::getProcessTriple(),
                                                          cis->getDiagnostics());
    drv->setTitle("myjitclangpp");
    drv->setCheckInputsExist(false);

    // this->mcis = cis;
    // this->mciv = civ;
    // this->mdrv = drv;

    static char *argv[] = {
        (char*)"myjitqtrunner", (char*)"flycode.cxx",
        (char*)"-fPIC", (char*)"-x", (char*)"c++", 
        //        (char*)"-I/usr/include/qt", (char*)"-I/usr/include/qt/QtCore",
        (char*)"-I/usr/lib/clang/3.5.0/include",
    };
    static int argc = 6;
    char **targv = argv;

    llvm::SmallVector<const char*, 16> drv_args(targv, targv + argc);
    drv_args.push_back("-S");

    clang::driver::Compilation *C = drv->BuildCompilation(drv_args);

    const clang::driver::JobList &Jobs = C->getJobs();
    const clang::driver::Command *Cmd = llvm::cast<clang::driver::Command>(*Jobs.begin());
    const clang::driver::ArgStringList &CCArgs = Cmd->getArguments();


    // clang::CompilerInvocation *civ = new clang::CompilerInvocation();
    bret = clang::CompilerInvocation::CreateFromArgs(*civ,
                                                          const_cast<const char**>(CCArgs.data()),
                                                          const_cast<const char**>(CCArgs.data()) + CCArgs.size(),
                                                          cis->getDiagnostics());
    cis->setInvocation(civ);

    cis->setPreprocessor(&pp);
    cis->setASTContext(pctx);
    cis->setFileManager(&pfm);
    cis->setSourceManager(&srcman);
    cis->setDiagnostics(&diag);
    // cis->setSema(&sema); // compile显示成功，但module是NULL

    // cis->createFileManager();
    // cis->createSourceManager(cis->getFileManager());
    // cis->createFrontendTimer();
    // cis->createASTContext(); // crash
    // cis->createModuleManager(); // crash

    // this->mcis->setSema(&unit->getSema());
    // const char *pcode = strdup(code.toLatin1().data());
    // const char *pcode = strdup("\n void abc() { QString(\"abcefg\").length(); } "); // ok
    const char *pcode = strdup("/*#include \"qthdrsrc.h\"*/\n void abc() { QString(\"abcefg\").length(); } ");
    llvm::MemoryBuffer *mbuf = llvm::MemoryBuffer::getMemBuffer(pcode);
    clang::PreprocessorOptions &ppOpt = cis->getPreprocessorOpts();
    ppOpt.addRemappedFile("flycode.cxx", mbuf);
    // 解析这段代码，结合已有的astfile查找标识符号。???
    // clang::ParseAST(unit->getSema());

    qDebug()<<cis->hasASTConsumer();// false
    // clang::ASTConsumer &cons = cis->getASTConsumer();
    // clang::ParseAST(pp, &cons, ctx);

    clang::EmitLLVMOnlyAction llvm_only_action;
    // clang::FrontendInputFile fif("./data/qthdrsrc.ast", clang::IK_AST);
    // llvm_only_action.setCurrentInput(fif, unit); // 不管用，不知道为什么。
    qDebug()<<"ast support:"<<llvm_only_action.hasASTFileSupport();
    qDebug()<<"what ctx:"<<cis->hasASTContext()<<&cis->getASTContext()<<pctx;
    qDebug()<<ctx.getExternalSource();

    bret = cis->ExecuteAction(llvm_only_action);
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

void testGenerateCode(clang::CodeGen::CodeGenModule &CGM, clang::GlobalDecl GD, llvm::Function *Fn,
                                   const clang::CodeGen::CGFunctionInfo &FnInfo) {
    using namespace clang;
    using namespace clang::CodeGen;
    const FunctionDecl *FD = cast<FunctionDecl>(GD.getDecl());
    qDebug()<<FD;

    // Check if we should generate debug info for this function.
    //     if (FD->hasAttr<NoDebugAttr>())
    //    DebugInfo = nullptr; // disable debug info indefinitely for this function

    FunctionArgList Args;
    QualType ResTy = FD->getReturnType();

    auto CurGD = GD;
    const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(FD);
    // if (MD && MD->isInstance()) {
    //     if (CGM.getCXXABI().HasThisReturn(GD))
    //         ResTy = MD->getThisType(getContext());
    //     CGM.getCXXABI().buildThisParam(*this, Args);
    // }

    // for (unsigned i = 0, e = FD->getNumParams(); i != e; ++i)
    //     Args.push_back(FD->getParamDecl(i));

    // if (MD && (isa<CXXConstructorDecl>(MD) || isa<CXXDestructorDecl>(MD)))
    //     CGM.getCXXABI().addImplicitStructorParams(*this, ResTy, Args);

    SourceRange BodyRange;
    if (Stmt *Body = FD->getBody()) BodyRange = Body->getSourceRange();
    auto CurEHLocation = BodyRange.getEnd();

    // Use the location of the start of the function to determine where
    // the function definition is located. By default use the location
    // of the declaration as the location for the subprogram. A function
    // may lack a declaration in the source code if it is created by code
    // gen. (examples: _GLOBAL__I_a, __cxx_global_array_dtor, thunk).
    SourceLocation Loc = FD->getLocation();

    // If this is a function specialization then use the pattern body
    // as the location for the function.
    if (const FunctionDecl *SpecDecl = FD->getTemplateInstantiationPattern())
        if (SpecDecl->hasBody(SpecDecl))
            Loc = SpecDecl->getLocation();

    // Emit the standard function prologue.
    // StartFunction(GD, ResTy, Fn, FnInfo, Args, Loc, BodyRange.getBegin());

    FunctionDecl *UnsizedDealloc = 
        FD->getCorrespondingUnsizedGlobalDeallocationFunction();
    bool ok = (UnsizedDealloc != NULL);
    qDebug()<<UnsizedDealloc<<ok;
    if (UnsizedDealloc) {
        qDebug()<<"error match if";
    }
    // Generate the body of the function.
    // PGO.assignRegionCounters(GD.getDecl(), CurFn);
    if (isa<CXXDestructorDecl>(FD)) {
        // EmitDestructorBody(Args);
        qDebug()<<"hhhhhhhhhhh";
    }
    else if (isa<CXXConstructorDecl>(FD)) {
        // EmitConstructorBody(Args);
        qDebug()<<"hhhhhhhhhhh";
    }
    // else if (getLangOpts().CUDA &&
    //          !CGM.getCodeGenOpts().CUDAIsDevice &&
    //          FD->hasAttr<CUDAGlobalAttr>()) {
    //     // CGM.getCUDARuntime().EmitDeviceStubBody(*this, Args);
    // }
    else if (isa<CXXConversionDecl>(FD) &&
             cast<CXXConversionDecl>(FD)->isLambdaToBlockPointerConversion()) {
        // The lambda conversion to block pointer is special; the semantics can't be
        // expressed in the AST, so IRGen needs to special-case it.
        // EmitLambdaToBlockPointerBody(Args);
        qDebug()<<"hhhhhhhhhhh";
    } else if (isa<CXXMethodDecl>(FD) &&
               cast<CXXMethodDecl>(FD)->isLambdaStaticInvoker()) {
        // The lambda static invoker function is special, because it forwards or
        // clones the body of the function call operator (but is actually static).
        // EmitLambdaStaticInvokeFunction(cast<CXXMethodDecl>(FD));
        qDebug()<<"hhhhhhhhhhh";
    } else if (FD->isDefaulted() && isa<CXXMethodDecl>(FD) &&
               (cast<CXXMethodDecl>(FD)->isCopyAssignmentOperator() ||
                cast<CXXMethodDecl>(FD)->isMoveAssignmentOperator())) {
        // Implicit copy-assignment gets the same special treatment as implicit
        // copy-constructors.
        // emitImplicitAssignmentOperatorBody(Args);
        qDebug()<<"hhhhhhhhhhh";
    } else if (Stmt *Body = FD->getBody()) {
        // EmitFunctionBody(Args, Body);
        qDebug()<<"hhhhhhhhhhh";
        // } else if (UnsizedDealloc != NULL) {
    } else if (FunctionDecl *UnsizedDealloc = 
               FD->getCorrespondingUnsizedGlobalDeallocationFunction()) {
        // Global sized deallocation functions get an implicit weak definition if
        // they don't have an explicit definition.
        // EmitSizedDeallocationFunction(*this, UnsizedDealloc);
        // EmitSizedDeallocationFunction(CGM, UnsizedDealloc);
        qDebug()<<"why hereeeeeeeeee"<<UnsizedDealloc<<(UnsizedDealloc != NULL);
    } else {
        qDebug()<<"hhhhhhhhhhh";
        llvm_unreachable("no definition for emitted function");
    }
}

// 不能正确生成ll代码
bool CompilerEngine::tryCompile_tpl(clang::ClassTemplateDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit)
{
    qDebug()<<"kind name:"<<decl->getDeclKindName();

    clang::CXXRecordDecl *rec_decl2 = decl->getTemplatedDecl();
    qDebug()<<"QTypedArrayData:"<<rec_decl2;
    clang::Decl *td = NULL;

    // for test;
    for (auto d: rec_decl2->methods()) {
        qDebug()<<d->getName().data();
        QString mthname = QString(d->getName().data());
        if (mthname == "sharedNull") {
            td = d;
            break;
        }
    }
    auto mthdecl = clang::cast<clang::CXXMethodDecl>(td);
    td->dumpColor();
    qDebug()<<mthdecl->getName().data();

    auto genmth = [](clang::CodeGen::CodeGenModule &cgm,
                     clang::CodeGen::CodeGenFunction &cgf,
                     clang::CXXMethodDecl *decl) -> bool {
        qDebug()<<cgm.getMangledName(decl).data();
        clang::CodeGen::CodeGenTypes &cgtypes = cgm.getTypes();

        const clang::CodeGen::CGFunctionInfo &FI = 
        cgtypes.arrangeGlobalDeclaration(decl);
                    
        llvm::FunctionType *FTy = cgtypes.GetFunctionType(FI);
        qDebug()<<"aaaaa";
        FTy->dump();
        qDebug()<<"aaaaa";

        clang::QualType retype = decl->getReturnType();
        qDebug()<<retype.getAsString().data();
        llvm::Type *lvtype = cgtypes.ConvertType(retype);
        llvm::Constant * v = cgm.GetAddrOfFunction(decl, FTy,
                                                   false, true);

        llvm::Function *f = llvm::cast<llvm::Function>(v);
        qDebug()<<"dbg func:"<<f->arg_size()
        << cgf.CapturedStmtInfo;
        // f->viewCFG();
        // clang::Stmt *stmt = decl->getBody();
        // clang::CodeGen::FunctionArgList alist;
        // f->addFnAttr(llvm::Attribute::AlwaysInline);
        cgm.setFunctionLinkage(decl, f);

        // cgm.getModule().dump();
        clang::CodeGen::CodeGenFunction(cgm).GenerateCode(clang::GlobalDecl(decl), f, FI);
        // cgf.GenerateCode(clang::GlobalDecl(decl), f, FI);

        return false;
    };

    clang::CompilerInstance ci;
    // ci.createASTContext();
    ci.createDiagnostics();
    ci.createFileManager();

    // clang::ASTContext &astctx = ci.getASTContext();
    clang::DiagnosticsEngine &diag = ci.getDiagnostics();
    llvm::LLVMContext &vmctx = llvm::getGlobalContext();
    llvm::Module mod("piecegen", vmctx);
    llvm::Module libmod("libcodes", vmctx);
    clang::CodeGenOptions &cgopt = ci.getCodeGenOpts();

    llvm::DataLayout dlo("e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128");

    clang::CodeGen::CodeGenModule cgmod(ctx, cgopt, mod, dlo, diag);

    qDebug()<<"heeeeee...";
    clang::CodeGen::CodeGenFunction cgf(cgmod, false);

    qDebug()<<cgmod.getMangledName(mthdecl).data();
    auto mgctx = ctx.createMangleContext();
    std::string stms;
    llvm::raw_string_ostream stmo(stms);
    mgctx->mangleCXXName(mthdecl, stmo);
    qDebug()<<stmo.str().c_str();

    decltype(mthdecl) mthdecl2;
    for (auto d: decl->specializations()) {
        for (auto rd: d->methods()) {
            QString mthname = QString(rd->getName().data());
            if (mthname != "sharedNull") {
                continue;
            }
            stmo.flush();
            stms.clear();
            mgctx->mangleCXXName(rd, stmo);
            // qDebug()<<mthname<<stmo.str().c_str();
            if (QString(stmo.str().c_str()) == "_ZN15QTypedArrayDataItE10sharedNullEv") {
                mthdecl2 = rd;
                mthdecl2->dumpColor();
            }
        }
    }

    if (0) {
        decltype(mthdecl) md = mthdecl;
        qDebug()<<md->hasBody()<<md->hasInlineBody()
                <<md->hasTrivialBody();
        for (auto d: md->redecls()) {
            qDebug()<<d;
            d->dumpColor();
        }
        auto s = md->getBody();
        s->dumpColor();
        auto a = md->getInstantiatedFromMemberFunction();
        // a->dumpColor();
        // qDebug()<<"aaaaaaaaaaaaa";
        // md->getTemplateInstantiationPattern()->dumpColor();
    }

    QDateTime btime = QDateTime::currentDateTime();
    // genmth(cgmod, cgf, mthdecl);

    {
        decltype(mthdecl) td = mthdecl2;
        decltype(cgmod) &cgm = cgmod;
        auto &cgtypes = cgmod.getTypes();
        llvm::Constant *v = cgm.GetAddrOfFunction(td, NULL, false, true);
        qDebug()<<v->getName().data();
        llvm::Function *f = clang::cast<llvm::Function>(v);
        const clang::CodeGen::CGFunctionInfo &FI = 
            cgtypes.arrangeGlobalDeclaration(td);
        // const clang::Type *Ty = td->getType().getTypePtr();
        // const clang::FunctionType *FT = clang::cast<clang::FunctionType>(Ty);
        // const clang::FunctionProtoType *FPT = clang::dyn_cast<clang::FunctionProtoType>(FT);
        // qDebug()<<Ty<<FT<<FPT;
        // auto canq =
        //     clang::CanQual<clang::FunctionProtoType>::CreateUnsafe(td->getType());
        // auto &FI2 = cgtypes.arrangeFreeFunctionType(canq);
        //     Ty = getTypes().ConvertType(cast<ValueDecl>(GD.getDecl())->getType());
        // cgtypes.ConvertType(clang::cast<clang::ValueDecl>(td)->getType());
        qDebug()<<cgm.getMangledName(clang::GlobalDecl(td)).data();
        qDebug()<<td<<td->getCorrespondingUnsizedGlobalDeallocationFunction();
        cgf.GenerateCode(td, f, FI);
        // testGenerateCode(cgm, clang::GlobalDecl(td), f, FI);
    }

    QDateTime etime = QDateTime::currentDateTime();
    qDebug()<<"gen func time:"<<btime.msecsTo(etime);

    mod.dump();
    
    return false;
}
