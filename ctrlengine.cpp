
#include <clang/AST/DeclCXX.h>
#include <llvm/IR/Module.h>

#include "frontengine.h"
#include "compilerengine.h"
#include "operatorengine.h"
#include "ctrlengine.h"
#include "clvm.h"
#include "clvmengine.h"
#include "invokestorage.h"


CtrlEngine::CtrlEngine()
{
    mfe = new FrontEngine();
    mce = new CompilerEngine();
}


CtrlEngine::~CtrlEngine()
{
}


void *CtrlEngine::vm_new(QString klass, QVector<QVariant> uargs)
{
    // 处理流程，
    // 使用fe获取这个类的定义CXXRecordDecl*结构
    // 使用fe确定适合当前参数的构造函数定义CXXConstructorDecl*结构
    // 使用ce生成该构造函数的ll代码。
    // 使用oe生成调用这个构造函数的ll代码。
    // 获取两段ll代码合并（如果需要的话）
    // 以module的方式传入vme执行
    
    mfe->loadPreparedASTFile();
    clang::CXXRecordDecl *rec_decl = mfe->find_class_decl(klass);
    qDebug()<<rec_decl;
    qDebug()<<"uargs:"<<uargs;
    clang::CXXConstructorDecl *ctor_decl = mfe->find_ctor_decl(rec_decl, klass, uargs);
    qDebug()<<ctor_decl;
    ctor_decl->dumpColor();

    QVector<QVariant> dargs;
    mfe->get_method_default_params(ctor_decl, dargs);
    
    // auto mod = mce->conv_ctor(mfe->getASTContext(), ctor_decl);
    auto mod = mce->conv_ctor2(mfe->getASTUnit(), ctor_decl, dargs);
    qDebug()<<mod<<mod->getDataLayout();
    // mce->conv_ctor(mfe->getASTContext(), ctor_decl);
    QString symname = mce->mangle_ctor(mfe->getASTContext(), ctor_decl);
    qDebug()<<mod<<symname;

    // 默认参数编译成IR    
    clang::FunctionDecl *jmt_decl = mfe->find_free_function("__jit_main_tmpl");
    qDebug()<<jmt_decl;
    jmt_decl->dumpColor();

    int cnter = -1;
    if (true)
    for (auto &v: dargs) {
        cnter ++;
        // qDebug()<<v<<v.type()<<(int)v.type()<<v.userType();
        int t = (int)v.type();
        if (v.type() != QMetaType::User) continue;
        if (v.userType() != EvalType::id) continue;
        mce->gen_darg(mod, v, cnter, jmt_decl);
        EvalType r = v.value<EvalType>();
        qDebug()<<v<<r.ve<<r.vv;
    }    

    OperatorEngine oe;
    void *kthis = calloc(oe.getClassAllocSize(klass), 1);
    qDebug()<<oe.getClassAllocSize(klass)<<kthis<<(int64_t)kthis<<dargs.count();

    // QString lamsym = oe.bind(mod, "_ZN7QStringC2Ev", kthis, uargs, dargs);
    QString lamsym = oe.bind(mod, symname, kthis, klass, uargs, dargs, false);
    qDebug()<<lamsym;
    
    Clvm *vm = new Clvm;
    if (vm == NULL) qFatal("kkkkkkkkk");
    auto gv = vm->execute2(mod, lamsym);
    qDebug()<<"gv:"<<llvm::GVTOP(gv);
    // QVector<clang::CXXMethodDecl*> mths = mfe->find_method_decls(rec_decl, klass_name, "fromLatin1");
    // if (mths.count() > 0) {eval type: QVariant(EvalType, )
    //     clang::CXXMethodDecl *mth = mths.at(0);
    //     mce->conv_method(mfe->getASTContext(), mth);
    // }
    qDebug()<<"======================";

    return kthis;
}

QVariant CtrlEngine::vm_call(void *kthis, QString klass, QString method, QVector<QVariant> uargs)
{
    // mfe->loadPreparedASTFile();
    clang::CXXRecordDecl *rec_decl = mfe->find_class_decl(klass);
    qDebug()<<rec_decl;
    clang::CXXMethodDecl *mth_decl = mfe->find_method_decl(rec_decl, klass, method, uargs);
    qDebug()<<mth_decl<<mth_decl->isStatic();
    mth_decl->dumpColor();

    QVector<QVariant> dargs;
    mfe->get_method_default_params(mth_decl, dargs);
    
    // auto mod = mce->conv_method(mfe->getASTContext(), mth_decl);
    auto mod = mce->conv_method2(mfe->getASTUnit(), mth_decl);
    qDebug()<<mod<<mod->getDataLayout();
    // mce->conv_ctor(mfe->getASTContext(), ctor_decl);
    QString symname = mce->mangle_method(mfe->getASTContext(), mth_decl);
    qDebug()<<mod<<symname;

    // 默认参数编译成IR
    clang::FunctionDecl *jmt_decl = mfe->find_free_function("__jit_main_tmpl");
    qDebug()<<jmt_decl;
    jmt_decl->dumpColor();
    // /*    
    int cnter = -1;
    if (true)
    for (auto &v: dargs) {
        cnter ++;
        // qDebug()<<v<<v.type()<<(int)v.type()<<v.userType();
        int t = (int)v.type();
        if (v.type() != QMetaType::User) continue;
        if (v.userType() != EvalType::id) continue;
        mce->gen_darg(mod, v, cnter, jmt_decl);
        EvalType r = v.value<EvalType>();
        qDebug()<<v<<r.ve<<r.vv;
    }
    // mod->dump();
    // */

    OperatorEngine oe;
    QString lamsym = oe.bind(mod, symname, kthis, klass, uargs, dargs, mth_decl->isStatic());
    qDebug()<<lamsym;
    
    Clvm *vm = new Clvm;
    auto gv = vm->execute2(mod, lamsym);
    qDebug()<<"gv:"<<llvm::GVTOP(gv);
    qDebug()<<"======================";
    
    return QVariant();
}

int CtrlEngine::vm_enum(QString klass, QString enum_name)
{
    mfe->loadPreparedASTFile();

    if (klass == "Qt") {
        int ev = mfe->get_qtns_enum(enum_name);
        return ev;
    } else {
        clang::CXXRecordDecl *rec_decl = mfe->find_class_decl(klass);
        qDebug()<<klass<<rec_decl;
        // rec_decl->dumpColor();
        int ev = mfe->get_class_enum(rec_decl, enum_name);
        return ev;
    }
    return -1;
}







