
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


void *CtrlEngine::vm_new(QString klass_name, QVector<QVariant> uargs)
{
    // 处理流程，
    // 使用fe获取这个类的定义CXXRecordDecl*结构
    // 使用fe确定适合当前参数的构造函数定义CXXConstructorDecl*结构
    // 使用ce生成该构造函数的ll代码。
    // 使用oe生成调用这个构造函数的ll代码。
    // 获取两段ll代码合并（如果需要的话）
    // 以module的方式传入vme执行
    
    mfe->loadPreparedASTFile();
    clang::CXXRecordDecl *rec_decl = mfe->find_class_decl(klass_name);
    qDebug()<<rec_decl;
    clang::CXXConstructorDecl *ctor_decl = mfe->find_ctor_decl(rec_decl, klass_name, uargs);
    qDebug()<<ctor_decl;
    ctor_decl->dumpColor();

    auto mod = mce->conv_ctor(mfe->getASTContext(), ctor_decl);
    qDebug()<<mod;
    // mce->conv_ctor(mfe->getASTContext(), ctor_decl);

    OperatorEngine oe;
    oe.bind(mod, "_ZN7QStringC2Ev", uargs);


    // QVector<clang::CXXMethodDecl*> mths = mfe->find_method_decls(rec_decl, klass_name, "fromLatin1");
    // if (mths.count() > 0) {
    //     clang::CXXMethodDecl *mth = mths.at(0);
    //     mce->conv_method(mfe->getASTContext(), mth);
    // }

    return 0;
}
