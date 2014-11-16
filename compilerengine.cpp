
#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include <clang/CodeGen/ModuleBuilder.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/CodeGen/BackendUtil.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Tool.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Sema/Sema.h>
#include <clang/Sema/Template.h>
#include <clang/AST/ASTLambda.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
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


#include "invokestorage.h"
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

// 从一个模块转移到另一个模块
bool irfunccpy(llvm::Module *smod, llvm::Function *sfun, llvm::Module *dmod, llvm::Function *dfun)
{

    return false;
}

// 起始decl，一般是一个函数或者方法的定义
clang::FunctionDecl*
CompilerEngine::find_callee_decl_by_symbol(clang::Decl *bdecl, QString callee_symbol)
{
    clang::Stmt *stmts = bdecl->getBody();
    QStack<clang::Stmt*> fexpr; // flat expr
    QVector<clang::CallExpr*> cexpr; // call expr
    QVector<clang::CXXTemporaryObjectExpr*> toexpr; // 临时对象生成语句，和call类似，这里会有一个symbol

    if (stmts != NULL) {
        qDebug()<<"Wooooo...."<<stmts;
        int cnter = 0;
        for (auto expr: stmts->children()) {
            qDebug()<<"e:"<<cnter++<<expr->getStmtClassName();
            // expr->dumpColor();
            fexpr.push(expr);
        }
    }
    
    qDebug()<<"flat expr count:"<<fexpr.count();
    while (!fexpr.isEmpty()) {
        clang::Stmt *s = fexpr.pop();
        int cnter = 0;
        for (auto expr: s->children()) {
            cnter++;
            fexpr.push(expr);
        }
        // qDebug()<<"push child:"<<cnter<<s->getStmtClassName();
        
        if (clang::isa<clang::CallExpr>(s)) {
            cexpr.append(clang::cast<clang::CallExpr>(s));
        }
        else if (llvm::isa<clang::CXXTemporaryObjectExpr>(s)) {
            toexpr.append(llvm::cast<clang::CXXTemporaryObjectExpr>(s));
        }
        else if (llvm::isa<clang::ReturnStmt>(s)) {
            // qDebug()<<"do smth."<<s;
            // s->dumpColor();
        } else {
            qDebug()<<"do smth."<<s<<s->getStmtClassName();
        }
    }
    qDebug()<<bdecl<<"call expr count:"<<cexpr.count();

    auto mgctx = bdecl->getASTContext().createMangleContext();
    for (auto expr: cexpr) {
        qDebug()<<"==========";
        // expr->dumpColor();
        auto d = expr->getCalleeDecl();
        // d->dumpColor();
        std::string str; llvm::raw_string_ostream stm(str);
        mgctx->mangleName(clang::cast<clang::NamedDecl>(d), stm);
        qDebug()<<"mangle name:"<<stm.str().c_str();
        if (stm.str() != callee_symbol.toStdString()) continue;

        if (clang::isa<clang::FunctionDecl>(d)) {
            return clang::cast<clang::FunctionDecl>(d);
        }
        assert(1==2);
        // for test
        if (clang::isa<clang::CXXMethodDecl>(d)) {
            auto d1 = clang::cast<clang::CXXMethodDecl>(d);
            this->conv_method(bdecl->getASTContext(), d1);
        }
    }

    // qDebug()<<"toexpr count:"<<toexpr.count();
    for (auto expr: toexpr) {
        // qDebug()<<"see toexpr's decl:"<<expr;
        expr->getConstructor()->dumpColor();
        return expr->getConstructor();
    }

    // for ctor's CtorInit
    if (llvm::isa<clang::CXXConstructorDecl>(bdecl)) {
        auto ctor_decl = llvm::cast<clang::CXXConstructorDecl>(bdecl);
        for (auto ci: ctor_decl->inits()) {
            auto e = ci->getInit();
            qDebug()<<"ctorinit..."<<e;
            e->dumpColor();
            auto d = this->find_callee_decl_by_symbol(bdecl, callee_symbol, e);
            if (d) return d;
        }
    }
    
    return NULL;
}

clang::FunctionDecl*
CompilerEngine::find_callee_decl_by_symbol(clang::Decl *bdecl, QString callee_symbol,
                                                clang::Stmt *bstmt)
{
    clang::Stmt *stmts = bstmt;
    QStack<clang::Stmt*> fexpr; // flat expr
    QVector<clang::CallExpr*> cexpr; // call expr
    QVector<clang::CXXConstructExpr*> ctor_expr;
    
    int cnter = 0;
    for (auto expr: stmts->children()) {
        qDebug()<<"e:"<<cnter++<<expr->getStmtClassName();
        // expr->dumpColor();
        fexpr.push(expr);
    }
    if (fexpr.count() == 0 && llvm::isa<clang::Expr>(bstmt)) {
        fexpr.push(bstmt);
    }

    qDebug()<<"flat expr count:"<<fexpr.count();
    auto mgctx = bdecl->getASTContext().createMangleContext();
    
    while (!fexpr.isEmpty()) {
        clang::Stmt *s = fexpr.pop();
        for (auto expr: s->children()) {
            fexpr.push(expr);
        }
        
        if (clang::isa<clang::CallExpr>(s)) {
            cexpr.append(clang::cast<clang::CallExpr>(s));
        }
        else if (clang::isa<clang::MemberExpr>(s)) {
        }
        else if (clang::isa<clang::DeclRefExpr>(s)) {
            auto d = llvm::cast<clang::DeclRefExpr>(s)->getDecl();
            qDebug()<<"dddddddref"<<d->getName().data();
            d->dumpColor();
            if (0) {
                std::string str; llvm::raw_string_ostream stm(str);
                mgctx->mangleName(clang::cast<clang::NamedDecl>(d), stm);
                qDebug()<<"mangle name:"<<stm.str().c_str();
            }
        }
        else if (llvm::isa<clang::CXXConstructExpr>(s)) {
            ctor_expr.append(llvm::cast<clang::CXXConstructExpr>(s));
        }
        else {
            qDebug()<<"stmt..."<<s<<s->getStmtClassName();
        }
    }
    if (cexpr.count() == 0 && llvm::isa<clang::CallExpr>(bstmt)) {
        cexpr.append(llvm::cast<clang::CallExpr>(bstmt));
    }
    qDebug()<<"call expr count:"<<cexpr.count()<<ctor_expr.count();

    for (auto expr: cexpr) {
        qDebug()<<"==========";
        // expr->dumpColor();
        auto d = expr->getCalleeDecl();
        // d->dumpColor();
        std::string str; llvm::raw_string_ostream stm(str);
        mgctx->mangleName(clang::cast<clang::NamedDecl>(d), stm);
        qDebug()<<"mangle name:"<<stm.str().c_str();
        if (stm.str() != callee_symbol.toStdString()) continue;

        if (clang::isa<clang::FunctionDecl>(d)) {
            return clang::cast<clang::FunctionDecl>(d);
        }
        assert(1==2);
        // for test
        if (clang::isa<clang::CXXMethodDecl>(d)) {
            auto d1 = clang::cast<clang::CXXMethodDecl>(d);
            this->conv_method(bdecl->getASTContext(), d1);
        }
    }

    for (auto expr: ctor_expr) {
        auto cd = expr->getConstructor();
        std::string str; llvm::raw_string_ostream stm(str);
        std::string str2; llvm::raw_string_ostream stm2(str2);
        mgctx->mangleCXXCtor(cd, clang::Ctor_Base, stm);
        mgctx->mangleCXXCtor(cd, clang::Ctor_Complete, stm2);
        qDebug()<<"mangle name:"<<stm.str().c_str()<<callee_symbol
                <<stm2.str().c_str();
        if (stm.str() == callee_symbol.toStdString()
            || stm2.str() == callee_symbol.toStdString()) {
            if (clang::isa<clang::FunctionDecl>(cd)) {
                return clang::cast<clang::FunctionDecl>(cd);
            }
        }
    }
    
    return NULL;
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

    
    for (auto fname: gfuns) {
        auto d = find_callee_decl_by_symbol(decl, fname);
        if (d == NULL) continue;
        if (clang::isa<clang::CXXMethodDecl>(d)) {
            auto d1 = get_decl_with_body(clang::cast<clang::CXXMethodDecl>(d));
            clang::CodeGen::CodeGenFunction cgf(cgmod);
            if (d1->isInlined()) {
                genmth(cgmod, cgf, d1);
            } else {
                genmth_decl(cgmod, cgf, d1, mtmod);                
            }
        }
        mod->dump();
        exit(-1);
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
// TODO 模板类中的静态方法支持
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

    QHash<QString, bool> noinlined; // 不需要生成define的symbol，在decl2def中使用。
    if (mth->isInlined()) {
        genmth(cgmod, *cgf, mth);
    } else {
        genmth_decl(cgmod, *cgf, mth, mtmod);
        noinlined[cgmod.getMangledName(mth).data()] = true;
    }

    qDebug()<<"dump module after gencode...";    
    mod->dump();

    decl2def(mod, ctx, cgmod, mth, 0, noinlined);

    qDebug()<<"dump module after all...";
    mod->dump();

    return mod;
}

// TODO 使用 MangleContext::MangleCtor 代替当前方式。
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

llvm::Module* 
CompilerEngine::conv_ctor2(clang::ASTUnit *unit, clang::CXXConstructorDecl *ctor,
                           QVector<QVariant> dargs)
{
    auto cu = this->createCompilerUnit(unit, ctor);
    cu->mdargs = dargs;
    this->mcus.insert(cu->mmod, cu);
    
    this->gen_ctor(cu);

    this->gen_undefs(cu);

    return cu->mmod;
    return 0;
}

llvm::Module* 
CompilerEngine::conv_method2(clang::ASTUnit *unit, clang::CXXMethodDecl *mth)
{
    auto cu = this->createCompilerUnit(unit, mth);
    this->mcus.insert(cu->mmod, cu);
    
    if (llvm::cast<clang::CXXMethodDecl>(cu->mbdecl)->isInlined()) {
        this->gen_method(cu);
    } else {
        this->gen_method_decl(cu);
    }
    qDebug()<<"base decl gen done.";

    this->gen_undefs(cu);
    
    int cnter = 100; // 0=>100, depcreated codes below
    while (cnter++ < 10) {
        this->find_undef_symbols(cu);
        qDebug()<<cu->mUndefSymbols.count();
        if (cu->mUndefSymbols.count() == 0) break;

        for (auto sym: cu->mUndefSymbols) {
            
            auto callee_decl = this->find_callee_decl_by_symbol(cu->mbdecl, sym);
            qDebug()<<"calee decl:"<<callee_decl
                    <<"is method:"<<llvm::isa<clang::CXXMethodDecl>(callee_decl);
            QString tsym = cu->mcgm->getMangledName(callee_decl).data();
            if (tsym == "_Z7qt_noopv") {
                callee_decl->dumpColor();
                this->gen_free_function(cu, callee_decl);
            }
            if (tsym == "_ZNK10QByteArray4sizeEv") {
                auto callee_decl_with_body = 
                    this->get_decl_with_body(llvm::cast<clang::CXXMethodDecl>(callee_decl));
                if (callee_decl_with_body->isInlined()) {
                    this->gen_method(cu, callee_decl_with_body);
                } else {
                    this->gen_method_decl(cu, callee_decl_with_body);
                }
            }
        }

        continue;
        // for test
        // auto callee_decl = this->find_callee_decl_by_symbol(cu->mbdecl, "_ZNK10QByteArray4sizeEv");
    }

    cu->mmod->dump();
    qDebug()<<"all gen done...";

    return cu->mmod;
    return 0;
}

bool CompilerEngine::gen_ctor(CompilerUnit *cu, clang::CXXConstructorDecl *yactor)
{
    // 转换到需要的参数类型
    auto ctor = yactor != NULL ? yactor : clang::cast<clang::CXXConstructorDecl>(cu->mbdecl);
    auto &cgmod = *(cu->mcgm);
    auto cgf = cu->mcgf;

    // 
    auto &cgtypes = cgmod.getTypes();
    // try ctor base , 能生成正确的Base代码了

    // 生成complete ctor, xxC1Exx
    {
        const clang::CodeGen::CGFunctionInfo &FIB = 
            cgtypes.arrangeCXXConstructorDeclaration(ctor, clang::Ctor_Complete);
        llvm::FunctionType *FTyB = cgtypes.GetFunctionType(FIB);
        llvm::Constant *ctor_base_val = 
            cgmod.GetAddrOfCXXConstructor(ctor, clang::Ctor_Complete, &FIB, true);
        llvm::Function *ctor_base_fn = clang::cast<llvm::Function>(ctor_base_val);
        clang::CodeGen::FunctionArgList alist;
        
        if (ctor->isInlined()) {
            cgmod.setFunctionLinkage(clang::GlobalDecl(ctor, clang::Ctor_Complete), ctor_base_fn);
            cgf->GenerateCode(clang::GlobalDecl(ctor, clang::Ctor_Complete), ctor_base_fn, FIB);
        } else {
            // QHash<QString, bool> noinlined; // 不需要生成define的symbol，在decl2def中使用。
            // noinlined[ctor_base_fn->getName().data()] = true;
            cu->mNoinlineSymbols[ctor_base_fn->getName().data()] = true;
        }
    }
        
    // 生成base ctor, xxC2Exx
    {
        const clang::CodeGen::CGFunctionInfo &FIB = 
            cgtypes.arrangeCXXConstructorDeclaration(ctor, clang::Ctor_Base);
        llvm::FunctionType *FTyB = cgtypes.GetFunctionType(FIB);
        llvm::Constant *ctor_base_val = 
            cgmod.GetAddrOfCXXConstructor(ctor, clang::Ctor_Base, &FIB, true);
        llvm::Function *ctor_base_fn = clang::cast<llvm::Function>(ctor_base_val);
        clang::CodeGen::FunctionArgList alist;
        if (ctor->isInlined()) {
            cgmod.setFunctionLinkage(clang::GlobalDecl(ctor, clang::Ctor_Base), ctor_base_fn);
            cgf->GenerateCode(clang::GlobalDecl(ctor, clang::Ctor_Base), ctor_base_fn, FIB);
        } else {
            // QHash<QString, bool> noinlined; // 不需要生成define的symbol，在decl2def中使用。
            // noinlined[ctor_base_fn->getName().data()] = true;
            cu->mNoinlineSymbols[ctor_base_fn->getName().data()] = true;
        }
    }


    return false;
}

// TODO 优化，改进，现有问题是，一个函数中如果多个这种类型的参数，则会不适用
bool CompilerEngine::gen_darg(llvm::Module *mod, QVariant &darg, int idx, clang::FunctionDecl *fd)
{
    CompilerUnit *cu = mcus.value(mod);
    
    /*
    for (auto &v: cu->mdargs) {
        if (v.type() != EvalType::id) continue;
        EvalType r = v.value<EvalType>();
        // cgmod.EmitConstantExpr(r.ve,
        this->gen_darg(cu, v);
    }
    */
    EvalType r = darg.value<EvalType>();
    auto expr = llvm::cast<clang::CXXConstructExpr>(r.ve);
    auto ctor = expr->getConstructor();
    this->gen_ctor(cu, ctor);
    qDebug()<<"hhhhhhhhh";
    ctor->dumpColor();

    // this->gen_free_function(cu, fd);
    
    // 对这种初始化，一般无法直接生成。
    auto v = cu->mcgm->EmitConstantExpr(expr, expr->getType());
    qDebug()<<v;

    // cu->mcgf->EmitCXXConstructLValue(expr);
    // qDebug()<<v;

    if (0) {
        qDebug()<<"hhhhhhhhhee";
        auto tv = cu->mcgf->EmitAnyExprToTemp(expr);
        r.vv = tv.getAggregateAddr();
        qDebug()<<"hhhhhhhhhee"<<tv.getAggregateAddr()<<tv.getScalarVal()
                <<tv.isScalar()<<tv.isComplex()<<tv.isAggregate();
        qDebug()<<darg;
        darg = QVariant::fromValue(EvalType(r.ve, r.vv));
        r.vv->setName("argxx");
        qDebug()<<darg<<r.vv->hasName()<<r.vv->getName().data();
        r.vv->dump();
    }
    if (0) {
        llvm::Value *mv = NULL;
        // mv = llvm::Constant::getNullValue(cu->mcgf->Int8PtrTy);
        // mv = cu->mcgf.Builder
        mv = cu->mcgf->CreateIRTemp(expr->getType());
        // cu->mcgf->EmitAnyExprToMem(expr, mv, expr->getType().getQualifiers(), true);
        // auto lv = cu->mcgf->Emit
        qDebug()<<mv;
        mv->dump();
    }

    
    auto ce = *expr->children();
    qDebug()<<"ffffffff";
    ce->dumpColor();
    if (0) {
        auto lv = cu->mcgf->EmitMaterializeTemporaryExpr(llvm::cast<clang::MaterializeTemporaryExpr>(ce));
        auto lvd = lv.getAddress();
        qDebug()<<lv.getAddress();
        lvd->dump();
        r.vv = lvd;
        darg = QVariant::fromValue(EvalType(r.ve, r.vv));
        r.vv->setName("argxx");
        
    }
    // auto lv = cu->mcgm->GetAddrOfGlobalTemporary(llvm::cast<clang::MaterializeTemporaryExpr>(ce), NULL);
    // qDebug()<<lv;

    if (1) {
        clang::CodeGen::CodeGenFunction cgf(*cu->mcgm, true);
        // cgf.EmitMaterializeTemporaryExpr(llvm::cast<clang::MaterializeTemporaryExpr>(ce));
        clang::CodeGen::FunctionArgList args;
        auto &cgtypes = cu->mcgm->getTypes();
        const clang::CodeGen::CGFunctionInfo &FI
            // = cgtypes.arrangeLLVMFunctionInfo();
            = cgtypes.arrangeFunctionDeclaration(fd);
        llvm::Constant *func = cu->mcgm->GetAddrOfFunction(fd);
        llvm::Function *func_fn = llvm::cast<llvm::Function>(func);
        
        cu->mcgf->StartFunction(clang::GlobalDecl(fd), fd->getReturnType(),
                                func_fn, FI, args);
    }

    if (1) {
        // auto lv = cu->mcgf->EmitLValue(expr);
        qDebug()<<"ck:"<<expr->getConstructionKind()
                <<ctor->isTrivial()<<ctor->isDefaultConstructor();
        // 为什么这个设置不管用呢？
        // expr->setConstructionKind(clang::CXXConstructExpr::CK_Complete);
        // expr->setConstructionKind(clang::CXXConstructExpr::CK_VirtualBase);
        // expr->setConstructionKind(clang::CXXConstructExpr::CK_NonVirtualBase);
        // expr->setConstructionKind(clang::CXXConstructExpr::CK_Delegating);
        qDebug()<<"ck:"<<expr->getConstructionKind();

        // ctor->setTrivial(false);
        // cu->mcgf->CurGD = clang::GlobalDecl(ctor, clang::Ctor_Base);// 不管用
        // qDebug()<<"ctor type:"<<cu->mcgf->CurGD.getCtorType();
        // expr->setElidable(false); // no
        // expr->setRequiresZeroInitialization(true); // no
        qDebug()<<"elidable:"<<expr->isElidable();
        
        auto lv = cu->mcgf->EmitCXXConstructLValue(expr);
        // qDebug()<<"ctor type:"<<cu->mcgf->CurGD.getCtorType();
        auto lvd = lv.getAddress();
        r.vv = lvd;
        lvd->setName(QString("toargx%1").arg(idx).toStdString());
        EvalType nr(r.ve, r.vv);
        // ctor->setElidable(false);
        nr.vf_base_name = this->mangle_ctor(cu->munit->getASTContext(), ctor);
        // darg = QVariant::fromValue(EvalType(r.ve, r.vv));
        darg = QVariant::fromValue(nr);
        
        qDebug()<<lvd;
        lvd->dump();
    }

    if (1) {
        // ret xxx
        cu->mcgf->FinishFunction();
    }

    qDebug()<<"fffffffff:";
    this->gen_undefs(cu, ctor, expr);
    
    mod->dump();
    // exit(0);
    
    return false;
}

bool CompilerEngine::gen_method(CompilerUnit *cu, clang::CXXMethodDecl *yamth)
{

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
    if (yamth) {
        return genmth(*cu->mcgm, *cu->mcgf, yamth);
    } else {
        return genmth(*cu->mcgm, *cu->mcgf, clang::cast<clang::CXXMethodDecl>(cu->mbdecl));
    }

    return false;
}

bool CompilerEngine::gen_method_decl(CompilerUnit *cu, clang::CXXMethodDecl *yamth)
{

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

    if (yamth) {
        cu->mNoinlineSymbols[QString(cu->mcgm->
                                     getMangledName(yamth)
                                     .data())]
            = true;
        return genmth_decl(*cu->mcgm, *cu->mcgf, yamth, mtmod);
    } else {
        cu->mNoinlineSymbols[QString(cu->mcgm->
                                     getMangledName(llvm::cast<clang::CXXMethodDecl>(cu->mbdecl))
                                     .data())]
            = true;
        return genmth_decl(*cu->mcgm, *cu->mcgf, llvm::cast<clang::CXXMethodDecl>(cu->mbdecl), mtmod);
    }

    return false;
}

bool CompilerEngine::gen_free_function(CompilerUnit *cu, clang::FunctionDecl *yafun)
{
    // 转换到需要的参数类型
    auto &cgmod = *(cu->mcgm);
    auto cgf = cu->mcgf;

    // 
    auto &cgtypes = cgmod.getTypes();
    // try ctor base , 能生成正确的Base代码了
    const clang::CodeGen::CGFunctionInfo &FIB = 
        cgtypes.arrangeFunctionDeclaration(yafun);
    llvm::FunctionType *FTyB = cgtypes.GetFunctionType(FIB);
    llvm::Constant *fun_val = 
        cgmod.GetAddrOfFunction(yafun, FTyB, false, false);
    llvm::Function *fun_fn = clang::cast<llvm::Function>(fun_val);
    clang::CodeGen::FunctionArgList alist;

    if (yafun->isInlined()) {
        cgmod.setFunctionLinkage(clang::GlobalDecl(yafun), fun_fn);
        cgf->GenerateCode(clang::GlobalDecl(yafun), fun_fn, FIB);
    } else {
        // QHash<QString, bool> noinlined; // 不需要生成define的symbol，在decl2def中使用。
        // noinlined[ctor_base_fn->getName().data()] = true;
        cu->mNoinlineSymbols[fun_fn->getName().data()] = true;
    }
    
    return false;
}

bool CompilerEngine::gen_undefs(CompilerUnit *cu, clang::FunctionDecl *yafun, clang::Stmt *yastmt)
{
    QVector<clang::FunctionDecl*> froms;
    froms.append(llvm::cast<clang::FunctionDecl>(cu->mbdecl));
    if (yafun) froms.append(yafun);
    
    int cnter = 0;
    while (cnter++ < 10) {
        this->find_undef_symbols(cu);
        qDebug()<<cu->mUndefSymbols.count();
        if (cu->mUndefSymbols.count() == 0) break;

        for (auto sym: cu->mUndefSymbols) {
            clang::FunctionDecl *callee_decl = NULL;
            qDebug()<<"froms:"<<froms;
            for (auto d: froms) {
                // 有可能d是没有body的Decl，所以查找不到。
                callee_decl = this->find_callee_decl_by_symbol(d, sym);
                if (callee_decl) break;
            }
            qDebug()<<"callee decl:"<<callee_decl<<sym<<froms.count();
            if (callee_decl == NULL && yastmt) {
                callee_decl = this->find_callee_decl_by_symbol(froms.at(0), sym, yastmt);
            }
            qDebug()<<"callee decl:"<<callee_decl<<sym<<froms.count();
            if (callee_decl == NULL) {
                // 如果没有找到，则一定是查找方法还有问题
                cu->mmod->dump();
                for (auto d: froms) {
                    qDebug()<<"decl dump:"<<d;
                    d->dumpColor();
                }
                qFatal("vvvvvvv"); // 在作为ruby模块的情况下，这个方便，不会生成太长的调用桟列表
                assert(1==2);
            }
            qDebug()<<"is method:"<<llvm::isa<clang::CXXMethodDecl>(callee_decl)
                    <<", is ctor:"<<llvm::isa<clang::CXXConstructorDecl>(callee_decl)
                    <<", is tmpl inst:"<<callee_decl->isTemplateInstantiation()
                    <<", is func tmpl spec:"<<callee_decl->isFunctionTemplateSpecialization()
                    <<", is global:"<<callee_decl->isGlobal()
                    <<", end";
            /*
              可以分为四种情况，
              第一种是函数的Qt函数，第二种是普通类方法，第三种是模板类的方法，第四种是模板函数（少）,
              也许还要加一种构造方法，ctor
             */
            QString tsym = cu->mcgm->getMangledName(callee_decl).data();
            Q_ASSERT(tsym == sym);
            if (callee_decl->isTemplateInstantiation()) {
                // maybe controll
                QStringList known_syms = {
                    "_ZN15QTypedArrayDataIcE10sharedNullEv",
                    "_ZN15QTypedArrayDataItE10sharedNullEv",
                    "_ZN15QTypedArrayDataIcE4dataEv",
                    "_ZN6QFlagsIN2Qt10WindowTypeEEC1EMNS2_7PrivateEi", // ctor
                    "_ZN6QFlagsIN2Qt13AlignmentFlagEEC1EMNS2_7PrivateEi",
                    "_ZNK14QScopedPointerI11QObjectData21QScopedPointerDeleterIS0_EEptEv",
                };
                if (known_syms.contains(tsym)) {
                    if (llvm::isa<clang::CXXConstructorDecl>(callee_decl)) {
                        this->instantiate_method(cu, llvm::cast<clang::CXXMethodDecl>(callee_decl));
                        this->gen_ctor(cu, llvm::cast<clang::CXXConstructorDecl>(callee_decl));
                    } else {
                        this->instantiate_method(cu, llvm::cast<clang::CXXMethodDecl>(callee_decl));
                        this->gen_method(cu, llvm::cast<clang::CXXMethodDecl>(callee_decl));
                    }
                } else {
                    qDebug()<<"unsupported tmpl inst..."<<tsym;
                }
            }
            else if (llvm::isa<clang::CXXConstructorDecl>(callee_decl)) {
                // maybe controll
                QStringList known_syms = {
                    "_ZN5QSizeC1Eii", "_ZN5QRectC1Eiiii",
                };
                if (known_syms.contains(tsym)) {
                    auto callee_decl_with_body = 
                        this->get_decl_with_body(llvm::cast<clang::CXXMethodDecl>(callee_decl));
                    callee_decl_with_body = callee_decl_with_body->isInlined() ?
                        callee_decl_with_body : llvm::cast<clang::CXXMethodDecl>(callee_decl);
                    if (callee_decl_with_body->isInlined()) {
                        this->gen_ctor(cu, llvm::cast<clang::CXXConstructorDecl>
                                       (callee_decl_with_body));
                    } else {
                        // this->gen_method_decl(cu, callee_decl_with_body);
                        qDebug()<<"leave it there:"<<tsym;
                        assert(1==2);
                    }
                } else {
                    qDebug()<<"unsupported ctor..."<<tsym;
                }
            }
            else if (llvm::isa<clang::CXXMethodDecl>(callee_decl)) {
                // maybe controll
                QStringList known_syms = {
                    "_ZNK10QByteArray4sizeEv", "_ZN10QArrayData10sharedNullEv",
                    "_ZN10QArrayData4dataEv", "_ZN7QWidget6resizeERK5QSize",
                    "_ZN7QWidget11setGeometryERK5QRect", "_ZNK5QRect6heightEv",
                    "_ZNK6QPoint1xEv", "_ZNK6QPoint1yEv", "_ZNK5QRect4sizeEv",
                    "_ZNK5QRect5widthEv",
                };
                if (known_syms.contains(tsym)) {
                    auto callee_decl_with_body = 
                        this->get_decl_with_body(llvm::cast<clang::CXXMethodDecl>(callee_decl));
                    if (callee_decl_with_body->isInlined()) {
                        this->gen_method(cu, callee_decl_with_body);
                    } else {
                        this->gen_method_decl(cu, callee_decl_with_body);
                    }
                } else {
                    qDebug()<<"unsupported method..."<<tsym;
                }
            }
            else if (callee_decl->isGlobal()) {
                // maybe controll
                QStringList known_syms = {
                    "_Z7qt_noopv", "_Z9qt_assertPKcS0_i",
                };
                if (known_syms.contains(tsym)) {
                    this->gen_free_function(cu, callee_decl);
                } else {
                    qDebug()<<"unsupported global..."<<tsym;                    
                }
            }
            else {
                qDebug()<<"unsupported symbol..."<<tsym;
            }

            if (0) {
            if (tsym == "_Z7qt_noopv") {
                callee_decl->dumpColor();
                this->gen_free_function(cu, callee_decl);
            }
            else if (tsym == "_Z9qt_assertPKcS0_i") {
                qDebug()<<"Oooooooo...";
                this->gen_free_function(cu, callee_decl);
            }
            else if (tsym == "_ZNK10QByteArray4sizeEv") {
                auto callee_decl_with_body = 
                    this->get_decl_with_body(llvm::cast<clang::CXXMethodDecl>(callee_decl));
                if (callee_decl_with_body->isInlined()) {
                    this->gen_method(cu, callee_decl_with_body);
                } else {
                    this->gen_method_decl(cu, callee_decl_with_body);
                }
            }
            else if (tsym == "_ZN10QArrayData10sharedNullEv") {
                auto callee_decl_with_body = 
                    this->get_decl_with_body(llvm::cast<clang::CXXMethodDecl>(callee_decl));
                if (callee_decl_with_body->isInlined()) {
                    this->gen_method(cu, callee_decl_with_body);
                } else {
                    this->gen_method_decl(cu, callee_decl_with_body);
                }
            }
            else if (tsym == "_ZN10QArrayData4dataEv") {
                auto callee_decl_with_body = 
                    this->get_decl_with_body(llvm::cast<clang::CXXMethodDecl>(callee_decl));
                if (callee_decl_with_body->isInlined()) {
                    this->gen_method(cu, callee_decl_with_body);
                } else {
                    this->gen_method_decl(cu, callee_decl_with_body);
                }
            }
            else if (tsym == "_ZN15QTypedArrayDataIcE10sharedNullEv") {
                qDebug()<<"tmpl mthod..."<<callee_decl;
                callee_decl->dumpColor();
                this->instantiate_method(cu, llvm::cast<clang::CXXMethodDecl>(callee_decl));
                this->gen_method(cu, llvm::cast<clang::CXXMethodDecl>(callee_decl));
                // cu->mmod->dump();
            }
            else if (tsym == "_ZN15QTypedArrayDataItE10sharedNullEv") {
                qDebug()<<"tmpl mthod..."<<callee_decl;
                callee_decl->dumpColor();
                this->instantiate_method(cu, llvm::cast<clang::CXXMethodDecl>(callee_decl));
                this->gen_method(cu, llvm::cast<clang::CXXMethodDecl>(callee_decl));
                // cu->mmod->dump();
            }
            else if (tsym == "_ZN15QTypedArrayDataIcE4dataEv") {
                qDebug()<<"tmpl mthod..."<<callee_decl;
                callee_decl->dumpColor();
                this->instantiate_method(cu, llvm::cast<clang::CXXMethodDecl>(callee_decl));
                this->gen_method(cu, llvm::cast<clang::CXXMethodDecl>(callee_decl));
                // cu->mmod->dump();
            } else {
                qDebug()<<"unsupported..."<<tsym;
            }
            }
            // TODO 可能会有多个last_decl被覆盖，需要更好的处理
            if (callee_decl) froms.append(callee_decl);
        }

        continue;
        // for test
        // auto callee_decl = this->find_callee_decl_by_symbol(cu->mbdecl, "_ZNK10QByteArray4sizeEv");
    }
    
    return false;
}

clang::CXXMethodDecl *
CompilerEngine::get_decl_with_body(clang::CXXMethodDecl *decl)
{
    if (decl->hasInlineBody()) return decl;
    int cnt = 0;
    for (auto rd: decl->redecls()) cnt++;
    if (cnt == 1) return decl;

    for (auto rd: decl->redecls()) {
        if (rd == decl) continue;
        return (decltype(decl))rd;
    }

    return 0;
}

bool CompilerEngine::find_undef_symbols(CompilerUnit *cu)
{
    cu->mUndefSymbols.clear();
    for (auto &f: cu->mmod->getFunctionList()) {
        QString fname = f.getName().data();
        qDebug()<<"name:"<<f.getName().data()<<f.size()<<"decl:"<<f.isDeclaration();
        if (!f.isDeclaration()) {
            continue;
        }
        if (cu->mNoinlineSymbols.contains(fname)) {
            continue;
        }

        // 如何判断是不是inline的呢。
        if (this->is_in_type_module(fname)) {
            // copy function from prepared module
            qDebug()<<"copying func:"<<fname;
            continue;
        }
        cu->mUndefSymbols.append(fname);
    }

    qDebug()<<"need gen funs:"<<cu->mUndefSymbols.count();

    return false;
}

bool CompilerEngine::is_in_type_module(QString symbol)
{
    return this->mtmod->getFunction(symbol.toLatin1().data()) != NULL;
}

CompilerUnit *
CompilerEngine::createCompilerUnit(clang::ASTUnit *unit, clang::NamedDecl *decl)
{
    clang::ASTContext &ctx = unit->getASTContext();
    CompilerUnit *cu = new CompilerUnit();
    cu->munit = unit;
    cu->mdecl = decl;
    cu->mcis = new clang::CompilerInstance();
    cu->mcis->createDiagnostics();
    cu->mcis->createFileManager();
    // cu->mcis->setASTContext(ctx);

    clang::DiagnosticsEngine &diag = cu->mcis->getDiagnostics();
    clang::CodeGenOptions &cgopt = cu->mcis->getCodeGenOpts();

    cu->mvmctx = new llvm::LLVMContext();
    cu->mmod = new llvm::Module("piecegen2", *cu->mvmctx);

    // crash了半天，原来是因为这地方需要一个&类型llvm::DataLayout&
    // llvm::DataLayout dlo("e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128");
    // dlo = *mtmod->getDataLayout();
    auto &dlo = *mtmod->getDataLayout();
    cu->mcgm = new clang::CodeGen::CodeGenModule(ctx, cgopt, *cu->mmod, dlo, diag);
    cu->mcgf = new clang::CodeGen::CodeGenFunction(*cu->mcgm);

    // 
    cu->mbdecl = this->get_decl_with_body(llvm::cast<clang::CXXMethodDecl>(decl));
    qDebug()<<cu->mdecl<<cu->mbdecl<<(cu->mdecl == cu->mbdecl)<<cu->mcgm;

    return cu;
}

bool CompilerEngine::destroyCompilerUnit(CompilerUnit *cu)
{
    return true;
}

// 已经能够支持Ctor
// instantate时，不需要考虑是Ctor_Base或Ctor_Complete
bool CompilerEngine::instantiate_method(CompilerUnit *cu, clang::CXXMethodDecl *tmpl_mthdecl)
{
    clang::CXXMethodDecl *mthdecl2 = tmpl_mthdecl;
    clang::ASTUnit *unit = cu->munit;
    clang::ASTContext &ctx = unit->getASTContext();

    auto &sema = unit->getSema();
    sema.Initialize();
    
    // Find the function body that we'll be substituting.
    const clang::FunctionDecl *PatternDecl = mthdecl2->getTemplateInstantiationPattern();
    clang::Stmt *Pattern = PatternDecl->getBody(PatternDecl);

    clang::MultiLevelTemplateArgumentList TemplateArgs =
        sema.getTemplateInstantiationArgs(mthdecl2, nullptr, false, PatternDecl);

    auto ascope = new clang::Scope(nullptr, 0, ctx.getDiagnostics());

    // 使用其中之一
    if (true) {
        sema.ActOnStartOfFunctionDef(ascope, mthdecl2); // because of next call has nullptr scope
        sema.InstantiateFunctionDefinition(mthdecl2->getLocation(), mthdecl2, true, true);        
    } else {
        auto rs = sema.SubstStmt(Pattern, TemplateArgs);
        qDebug()<<rs.isInvalid()<<rs.get();
        rs.get()->dumpColor();
        // sema.ActOnFinishFunctionBody(mthdecl2, Pattern);
        mthdecl2->setBody(rs.get());
    }
    sema.PrintStats();
    
    return false;
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
bool CompilerEngine::tryCompile_tpl(clang::ClassTemplateDecl *decl, clang::ASTContext &ctx,
                                    clang::ASTUnit *unit)
{
    qDebug()<<"kind name:"<<decl->getDeclKindName();

    clang::CXXRecordDecl *rec_decl2 = decl->getTemplatedDecl();
    qDebug()<<"QTypedArrayData:"<<rec_decl2;
    clang::Decl *td = NULL;

    // for test;先查找到方法
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
    mthdecl->getBody()->dumpColor();
    
    qDebug()<<mthdecl->getName().data()<<mthdecl->clang::Decl::getDeclKindName()
            <<llvm::isa<clang::FunctionTemplateDecl>(mthdecl); // not FunctionTemplateDecl
    qDebug()<<mthdecl->hasBody()<<mthdecl->hasInlineBody()<<mthdecl->isInlined()
            <<mthdecl->isFunctionTemplateSpecialization()
            <<mthdecl->isTemplateDecl()
            <<mthdecl->isFunctionOrFunctionTemplate()
            <<mthdecl->getTemplatedKind();


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
        qDebug()<<"===========";
        s->dumpColor();
        auto a = md->getInstantiatedFromMemberFunction();
        // a->dumpColor();
        // qDebug()<<"aaaaaaaaaaaaa";
        // md->getTemplateInstantiationPattern()->dumpColor();
    }

    // 测试，先特化，后查找method
    if (false) {
        clang::ClassTemplateSpecializationDecl *ctsd;
        int cnter = 0;
        for (auto sd: decl->specializations()) {
            qDebug()<<cnter++<<sd;
            int jcnter = 0;
            auto &al = sd->getTemplateArgs();
            for (int j = 0; j < al.size(); j++) {
                qDebug()<<jcnter++<<al.get(j).getAsType().getAsString().data();
                QString tplat = al.get(j).getAsType().getAsString().data();
                if (tplat == "char") {
                    ctsd = sd;
                    clang::TemplateArgument a(al.get(j).getAsType());
                    void *inspos = nullptr;
                }
            }
        }
        qDebug()<<"==============";
        ctsd->dumpColor();
        std::vector<clang::TemplateArgument> targs;
        clang::CXXMethodDecl *mth3 = NULL;
        for (auto md: ctsd->methods()) {
            QString mthname = md->getName().data();
            if (mthname == "sharedNull") {
                mth3 = md;
                // 也不管用啊
                mthdecl->setInstantiationOfMemberFunction(mth3,
                                                       clang::TemplateSpecializationKind
                                                       ::TSK_ExplicitInstantiationDefinition);
            }
        }
        qDebug()<<mth3;
        mth3->dumpColor();
        mthdecl2 = mth3;
        int kcnter = 0;
        for (auto rd: mth3->redecls()) {
            qDebug()<<kcnter++<<rd;
        }

        qDebug()<<mth3->hasBody()<<mth3->hasInlineBody()<<mth3->isInlined()
                <<mth3->isFunctionTemplateSpecialization()
                <<mth3->isTemplateDecl()
                <<mth3->isFunctionOrFunctionTemplate()
                <<mth3->getTemplatedKind();


        // mth3->setBody(mthdecl->getBody()); // 强制设计body，但body中也有模板参数，没有特化的。
        // mth3->dumpColor();
        
    }

    if (true) {
        tryTransform(decl, ctx, unit, mthdecl, mthdecl2);        
        // tryTransform2(decl, ctx, unit, mthdecl, mthdecl2);
        // tryTransform3(decl, ctx, unit, mthdecl, mthdecl2);
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
        qDebug()<<td<<td->getCorrespondingUnsizedGlobalDeallocationFunction()
                <<td->isLambdaStaticInvoker()<<td->isInstance(); // 0x12345, 0x0, false
        cgf.GenerateCode(td, f, FI);
        // testGenerateCode(cgm, clang::GlobalDecl(td), f, FI);
    }

    QDateTime etime = QDateTime::currentDateTime();
    qDebug()<<"gen func time:"<<btime.msecsTo(etime);

    mod.dump();
    
    return false;
}

/*
  使用sema自带的模板transform实现模板推导
 */
bool CompilerEngine::tryTransform(clang::ClassTemplateDecl *decl,
                                  clang::ASTContext &ctx, clang::ASTUnit *unit,
                                  clang::CXXMethodDecl *mth, clang::CXXMethodDecl *mth2)
{
    clang::CXXMethodDecl *mthdecl = mth;
    clang::CXXMethodDecl *mthdecl2 = mth2;

    
    if (true) {
        qDebug()<<"==============";        
        mthdecl2->dumpColor();
        qDebug()<<unit->hasSema();
        auto &sema = unit->getSema();
        sema.Initialize();
        qDebug()<<&sema.getASTContext()<<&ctx; // (==)

        std::string locstr = mthdecl2->getLocation().printToString(ctx.getSourceManager());
        qDebug()<<locstr.c_str();
        // ActOnStartOfFunctionDef(nullptr, mthdecl2, &sema, ctx);
        // sema.ActOnStartOfFunctionDef(nullptr, mthdecl2);
        // sema.InstantiateFunctionDefinition(mthdecl2->getLocation(), mthdecl2, true, true);

               
        // Find the function body that we'll be substituting.
        const clang::FunctionDecl *PatternDecl = mthdecl2->getTemplateInstantiationPattern();
        clang::Stmt *Pattern = PatternDecl->getBody(PatternDecl);
        qDebug()<<"hhhhhhhhh"<<Pattern;
        // Pattern->dumpColor();
        
        bool MergeWithParentScope = false;
        if (clang::CXXRecordDecl *Rec = clang::dyn_cast<clang::CXXRecordDecl>(mthdecl2->getDeclContext())) {
            MergeWithParentScope = Rec->isLocalClass();
        }
        qDebug()<<MergeWithParentScope;
        
        clang::LocalInstantiationScope Scope(sema, MergeWithParentScope);

        clang::MultiLevelTemplateArgumentList TemplateArgs =
            sema.getTemplateInstantiationArgs(mthdecl2, nullptr, false, PatternDecl);

        // If this is a constructor, instantiate the member initializers.
        // if (const CXXConstructorDecl *Ctor =
        //     dyn_cast<CXXConstructorDecl>(PatternDecl)) {
        //     InstantiateMemInitializers(cast<CXXConstructorDecl>(Function), Ctor,
        //                                TemplateArgs);
        // }

        // Instantiate the function body.
        // StmtResult Body = sema.SubstStmt(Pattern, TemplateArgs);

        qDebug()<<mthdecl2->getTranslationUnitDecl();
        qDebug()<<mthdecl->getTranslationUnitDecl();
        
        qDebug()<<"==============";
        mthdecl2->dumpColor();
        qDebug()<<"==============";
        qDebug()<<ctx.getTranslationUnitDecl()->isTranslationUnit(); // true
        qDebug()<<sema.getCurFunction();
        sema.PushFunctionScope();
        auto scope = sema.getScopeForContext(decl->getDeclContext());
        auto ascope = new clang::Scope(nullptr, 0, ctx.getDiagnostics());
        // sema.ActOnTranslationUnitScope(ascope);
        qDebug()<<scope<<decl->getDeclContext()<<decl->getDeclContext()->getPrimaryContext();
        // sema.ActOnEndOfTranslationUnit();
        sema.PrintStats();

        qDebug()<<TemplateArgs.getNumLevels();
        auto arglist = TemplateArgs.getInnermost();
        for (auto a: arglist) {
            qDebug()<<a.getAsType().getAsString().data();
        }
        // sema.PushFunctionScope();
        qDebug()<<sema.CurrentInstantiationScope;
        qDebug()<<"curscope:"<<scope<<sema.getCurScope();
        sema.ActOnStartOfFunctionDef(ascope, mthdecl2); // because of next call has nullptr scope
        qDebug()<<"curscope:"<<scope<<sema.getCurScope();        
        sema.InstantiateFunctionDefinition(mthdecl2->getLocation(), mthdecl2, true, true);        
        // auto rs = sema.SubstStmt(Pattern, TemplateArgs);
        // qDebug()<<rs.isInvalid()<<rs.get();
        // rs.get()->dumpColor();
        // // sema.ActOnFinishFunctionBody(mthdecl2, Pattern);
        // mthdecl2->setBody(rs.get());
        // qDebug()<<mthdecl2->getDeclContext();
        sema.PrintStats();
        
        qDebug()<<"hhhhhhhhhhhhhhh";
        clang::TemplateDeclInstantiator instord(sema, mthdecl->getDeclContext(), TemplateArgs);
        qDebug()<<"hhhhhhhhhhhhhhh";        
        instord.InitMethodInstantiation(mthdecl2, mthdecl);
        qDebug()<<"hhhhhhhhhhhhhhh";
        auto nd = instord.VisitCXXMethodDecl(mthdecl, nullptr, true);
        qDebug()<<"okkkkkkkkk>???"<<nd;
        nd->dumpColor();
        mthdecl2->dumpColor();
        // auto cd2 = instor.TransformDefinition(mthdecl->getLocation(), mthdecl2);
        // qDebug()<<cd2;
        // cd2->dumpColor();
    }
    // exit(-1);
    return false;
}

/*
  自己实现的简单的模板类型替换
  已经能够转换cast<模板类型>的表达式了
 */
bool CompilerEngine::tryTransform2(clang::ClassTemplateDecl *decl,
                                   clang::ASTContext &ctx, clang::ASTUnit *unit,
                                   clang::CXXMethodDecl *mth, clang::CXXMethodDecl *mth2)
{
    using namespace clang;
    // 难度还是比较大的。
    auto simple_transform = [](clang::CompoundStmt *tmpl, clang::CXXMethodDecl *mth2) -> bool {
        int cnter = 0;
        for (auto cs: tmpl->children()) {
            if (cnter == 0) {
            } else {
                mth2->setBody(cs);
            }
            qDebug()<<cnter++<<cs<<cs->getStmtClassName();
        }

        auto tmpl2 = mth2->getBody();
        tmpl2->dumpColor();
        
        QStack<clang::Stmt*> ss;
        ss.push(tmpl2);
        while (!ss.isEmpty()) {
            auto s = ss.pop();
            if (llvm::isa<Expr>(s)) {
                auto e = llvm::cast<Expr>(s);
                clang::QualType t = e->getType();
                qDebug()<<t.getAsString().data()
                        <<t->getTypeClassName()
                        <<t->isIncompleteType()
                        <<t->isDependentType();
            }
            qDebug()<<s->getStmtClassName();
            for (auto cs: s->children()) {
                ss.push(cs);
            }
        }
        qDebug()<<ss.count();
       
        for (auto cs: tmpl2->children()) {
            qDebug()<<cs<<cs->getStmtClassName();
            if (llvm::isa<ExplicitCastExpr>(cs)) {
                auto e = llvm::cast<Expr>(cs);
                auto ce = llvm::cast<ExplicitCastExpr>(cs);
                clang::QualType ctot = llvm::cast<ExplicitCastExpr>(cs)->getTypeAsWritten();
                qDebug()<<ctot.getAsString().data()
                        <<ctot->getTypeClassName()
                        <<ctot->isIncompleteType()
                        <<ctot->isDependentType();
                clang::QualType ctot2 = ctot->getPointeeType();
                qDebug()<<ctot2.getAsString().data()
                        <<ctot2->getTypeClassName();
                e->setType(mth2->getReturnType()); // 这个管用，但对不对呢
                qDebug()<<"transformed expr:";
                e->dumpColor();
                continue;
                const clang::InjectedClassNameType *injty = ctot2->getAs<clang::InjectedClassNameType>();
                clang::QualType injty2 = injty->getInjectedSpecializationType();
                qDebug()<<injty2.getAsString().data()
                        <<injty2->getTypeClassName();
                auto we =ce->getSubExprAsWritten(); // subexpr即要cast的表达式部分，()内的部分
                we->dumpColor();
                qDebug()<<ce->path_size();
            }
        }
        
        return false;
    };
    
    // 测试，先查找method,再特化
    if (true) {
        clang::CXXMethodDecl *mthdecl = mth;
        clang::CXXMethodDecl *mthdecl2 = mth2;
        auto btmpl = llvm::cast<clang::CompoundStmt>(mthdecl->getBody());
        simple_transform(btmpl, mthdecl2);
    }

    return false;
}



