
#include <clang/AST/DeclCXX.h>

#include "frontengine.h"
#include "compilerengine.h"
#include "operatorengine.h"
#include "ctrlengine.h"
#include "clvm.h"
#include "clvmengine.h"

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
    qDebug()<<uargs;
    clang::CXXConstructorDecl *ctor_decl = mfe->find_ctor_decl(rec_decl, klass, uargs);
    qDebug()<<ctor_decl;
    ctor_decl->dumpColor();

    // auto mod = mce->conv_ctor(mfe->getASTContext(), ctor_decl);
    auto mod = mce->conv_ctor2(mfe->getASTUnit(), ctor_decl);
    qDebug()<<mod;// <<mod->getDataLayout();
    // mce->conv_ctor(mfe->getASTContext(), ctor_decl);
    QString symname = mce->mangle_ctor(mfe->getASTContext(), ctor_decl);
    qDebug()<<mod<<symname;

    OperatorEngine oe;
    QVector<QVariant> dargs;
    mfe->get_method_default_params(ctor_decl, dargs);
    void *kthis = calloc(oe.getClassAllocSize(klass), 1);
    qDebug()<<oe.getClassAllocSize(klass)<<kthis<<(int64_t)kthis<<dargs.count();

    // QString lamsym = oe.bind(mod, "_ZN7QStringC2Ev", kthis, uargs, dargs);
    QString lamsym = oe.bind(mod, symname, kthis, klass, uargs, dargs, false);
    qDebug()<<lamsym;
    Clvm *vm = new Clvm;
    auto gv = vm->execute2(mod, lamsym);
    qDebug()<<"gv:"<<llvm::GVTOP(gv);
    // QVector<clang::CXXMethodDecl*> mths = mfe->find_method_decls(rec_decl, klass_name, "fromLatin1");
    // if (mths.count() > 0) {
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

    // auto mod = mce->conv_method(mfe->getASTContext(), mth_decl);
    auto mod = mce->conv_method2(mfe->getASTUnit(), mth_decl);
    qDebug()<<mod;// <<mod->getDataLayout();
    // mce->conv_ctor(mfe->getASTContext(), ctor_decl);
    QString symname = mce->mangle_method(mfe->getASTContext(), mth_decl);
    qDebug()<<mod<<symname;

    OperatorEngine oe;
    QVector<QVariant> dargs;
    mfe->get_method_default_params(mth_decl, dargs);
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
    clang::CXXRecordDecl *rec_decl = mfe->find_class_decl(klass);
    qDebug()<<klass<<rec_decl;
    // rec_decl->dumpColor();
    
    int ev = mfe->get_class_enum(rec_decl, enum_name);

    return ev;
    return -1;
}







