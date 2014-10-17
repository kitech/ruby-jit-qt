
#include <clang/AST/DeclCXX.h>

#include "frontengine.h"
#include "compilerengine.h"
#include "operatorengine.h"
#include "ctrlengine.h"


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

    // mce->conv_ctor(mfe->getASTContext(), ctor_decl);
    // mce->conv_ctor(mfe->getASTContext(), ctor_decl);

    clang::CXXRecordDecl *rec_decl2 = mfe->find_class_decl("QTypedArrayData");
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

    td->dumpColor();

    return 0;
}
