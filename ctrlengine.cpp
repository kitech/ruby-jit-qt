#include "fix_clang_undef_eai.h"
#include <QtWidgets>

#include <clang/AST/DeclCXX.h>
#include <llvm/IR/Module.h>

#include "frontengine.h"
#include "compilerengine.h"
#include "operatorengine.h"
#include "ctrlengine.h"
#include "clvm.h"
#include "clvmengine.h"
#include "invokestorage.h"
#include "qtobjectmanager.h"
#include "ruby.hpp"

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
    if (symname.indexOf("LayoutC") != -1) {
        symname = symname.replace("C2", "C1");
    }

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
    memset(kthis, 0, oe.getClassAllocSize(klass));
    qDebug()<<oe.getClassAllocSize(klass)<<kthis<<(int64_t)kthis<<dargs.count();

    // QString lamsym = oe.bind(mod, "_ZN7QStringC2Ev", kthis, uargs, dargs);
    QString lamsym = oe.bind(mod, symname, klass, uargs, dargs, false, kthis);
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

// TODO 也许可以放在其他源文件中，专门用作类型转换的marshall中
QVariant GV2Variant(llvm::GenericValue gv, clang::FunctionDecl *decl, void *kthis)
{
    RB_VALUE rv = 0;
    clang::QualType rty = decl->getReturnType();
    // qDebug()<<"return type class:"<<rty->getTypeClassName();
    // rty->dump();
    if (rty->isVoidType()) return QVariant();

    if (rty->isBooleanType()) {
        bool bv = gv.Untyped[0] == 1;
        rv = bv ? Qtrue : Qfalse;
        qDebug()<<"boolean..."<<rv<<gv.Untyped[0];
    }
    else if (rty->isIntegralOrEnumerationType()) {
        rv = INT2NUM((qlonglong)llvm::GVTOP(gv));
        qDebug()<<"iore..."<<rv;
    }
    else if (rty->isIntegerType()) {
        rv = INT2NUM((qlonglong)llvm::GVTOP(gv));
        qDebug()<<"int..."<<rv;        
    }
    else if (rty->isFloatingType()) {
        double dv = 0;
        dv = gv.DoubleVal;
        rv = rb_float_new(dv);
        qDebug()<<"float..."<<rv<<dv;
        // qDebug()<<gv.Untyped[0]<<gv.Untyped[1]<<gv.Untyped[2]<<gv.Untyped[3];
    }
    else if (rty->isPointerType()) {
        //
        void *vv = llvm::GVTOP(gv);
        rv = Qom::inst()->getObject(vv);
        if (rv == 0) {
            qDebug()<<"unknown pointer type return:"<<vv;
        }
        qDebug()<<"x*...";        
    }
    // 类内类,像QMetaObject::Connection            
    else if (rty->isRecordType() && llvm::isa<clang::ElaboratedType>(rty.getTypePtr())) {
        auto rec_decl = rty->getAsCXXRecordDecl();
        qDebug()<<"not supported elaborated record type:"<<rec_decl->getName().data()
                <<rty->getTypeClassName()
                <<rty->isElaboratedTypeSpecifier()
                <<llvm::isa<clang::ElaboratedType>(rty.getTypePtr());
        // exit(0);qFatal("sttttttttt");
    }
    // sret type, 一般是类对象，并且是非引用或者指针。
    else if (rty->isRecordType()) {
        qDebug()<<"record...";
        auto rec_decl = rty->getAsCXXRecordDecl();
        rec_decl->dumpColor();
        qDebug()<<rec_decl->getName().data();

        VALUE modval = rb_const_get(rb_cObject, rb_intern("Qt5"));
        // VALUE clsval = rb_const_get(modval, rb_intern("QString"));
        VALUE clsval = rb_const_get(modval, rb_intern(rec_decl->getName().data()));
        qDebug()<<"kv:"<<TYPE(clsval)<<",,";
        VALUE retobj = rb_class_new_instance(0, 0, clsval); // 参数个数可能不适用。
        rv = retobj;
        qDebug()<<"old ci:"<<Qom::inst()->getObject(retobj);
        Qom::inst()->addObject(retobj, llvm::GVTOP(gv));
        qDebug()<<"new ci:"<<Qom::inst()->getObject(retobj);
    }
    // 引用类型返回值，如QString &
    else if (rty->isLValueReferenceType()) {
        qDebug()<<"lvalue ref:"<<"x&...";
        void *vv = llvm::GVTOP(gv);
        rv = Qom::inst()->getObject(vv);
        if (rv == 0) {
            qDebug()<<"unknown reference type return:"<<vv;
        }
    }
    else {
        qDebug()<<"unknown type return:"<<llvm::GVTOP(gv)
                <<rty->getTypeClassName();
        return QVariant();
    }
    
    return QVariant(rv);
}


QVariant CtrlEngine::vm_call(void *kthis, QString klass, QString method, QVector<QVariant> uargs)
{
    // hotfix
    if (klass == "QGridLayout"
        && (method == "addWidget" || method == "addLayout")) {
        // return this->vm_call_hotfix(kthis, klass, method, uargs);
    }
    
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
    QString lamsym = oe.bind(mod, symname, klass, uargs, dargs, mth_decl->isStatic(), kthis);
    qDebug()<<lamsym;
    
    Clvm *vm = new Clvm;
    auto gv = vm->execute2(mod, lamsym);
    qDebug()<<"gv:"<<llvm::GVTOP(gv)<<(qlonglong)llvm::GVTOP(gv)<<gv.IntVal.getZExtValue();
    qDebug()<<"======================";

    QVariant rv = GV2Variant(gv, mth_decl, kthis);
    return rv;
    return QVariant();
}

QVariant CtrlEngine::vm_static_call(QString klass, QString method, QVector<QVariant> uargs)
{
    qDebug()<<"aaaaaa";
    mfe->loadPreparedASTFile();
    clang::CXXRecordDecl *rec_decl = mfe->find_class_decl(klass);
    qDebug()<<rec_decl;
    clang::CXXMethodDecl *mth_decl = mfe->find_static_method_decl(rec_decl, klass, method, uargs);
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

    OperatorEngine oe;
    QString lamsym = oe.bind(mod, symname, klass, uargs, dargs, mth_decl->isStatic(), NULL);
    qDebug()<<lamsym;
    
    Clvm *vm = new Clvm;
    auto gv = vm->execute2(mod, lamsym);
    qDebug()<<"gv:"<<llvm::GVTOP(gv)<<gv.IntVal.getZExtValue();
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

QString CtrlEngine::vm_qdebug(void *kthis, QString klass)
{
    QString str;
    mfe->loadPreparedASTFile();
    
    // _Zls6QDebugRK9QBitArray
    QString symtpl = "_Zls6QDebugRK%1%2";
    QString symname = symtpl.arg(klass.length()).arg(klass);

    QBitArray *ba = new QBitArray();
    QThread *th = new QThread();
    void *vth = th;
    qDebug()<<ba<<th<<vth;

    QString stc; // stream container
    QDebug dm(&stc);
    
    clang::FunctionDecl *fun_decl = mfe->find_free_function2(symname);
    if (fun_decl) {
        // fun_decl->dumpColor();

        QVector<QVariant> dargs;
        // mfe->get_method_default_params(mth_decl, dargs);
        dargs << QVariant() << QVariant();

        QVector<QVariant> uargs;
        uargs.append(QVariant::fromValue((void*)(&dm)));
        uargs.append(QVariant::fromValue(kthis));
        
        auto mod = mce->conv_function2(mfe->getASTUnit(), fun_decl);
        qDebug()<<mod<<mod->getDataLayout();
        // mce->conv_ctor(mfe->getASTContext(), ctor_decl);
        // QString symname = mce->mangle_method(mfe->getASTContext(), mth_decl);
        qDebug()<<mod<<symname;

        OperatorEngine oe;
        QString lamsym = oe.bind(mod, symname, klass, uargs, dargs, false, NULL);
        qDebug()<<lamsym;
    
        Clvm *vm = new Clvm;
        auto gv = vm->execute2(mod, lamsym);
        qDebug()<<"gv:"<<llvm::GVTOP(gv)<<gv.IntVal.getZExtValue();
        qDebug()<<"======================";
    
        // qDebug()<<stc;
        str = stc;
    } else {
        if (klass == "QString") {
            dm << *(QString*)kthis;
            return stc;
        } else if (klass == "QByteArray") {
            dm << *(QByteArray*)kthis;
            return stc;
        }
        char buf[64] = {0};
        dm << kthis;
        snprintf(buf, sizeof(buf)-1, "%s(%s)", klass.toLatin1().data(), stc.trimmed().toLatin1().data());
        str = QString(buf);
        // qDebug()<<str;
    }

    return str;
}

/*
  不是因为使用了QLayoutC2
      
*/
QVariant
CtrlEngine::vm_call_hotfix(void *kthis, QString klass, QString method, QVector<QVariant> uargs)
{
    QGridLayout *qci = (QGridLayout*)kthis;
    QFlags<Qt::AlignmentFlag> f(0);
    if (uargs.count() == 4) {
        f = QFlags<Qt::AlignmentFlag>(uargs.at(3).toInt());
    } else if (uargs.count() == 6) {
        f = QFlags<Qt::AlignmentFlag>(uargs.at(5).toInt());
    }
    if (method == "addWidget") {
        void *va = uargs.at(0).value<void*>();
        QWidget *wa = (QWidget*)va;
        if (uargs.count() == 5 || uargs.count() == 6)
            qci->addWidget(wa, uargs.at(1).toInt(), uargs.at(2).toInt(),
                           uargs.at(3).toInt(), uargs.at(4).toInt(), f);
        if (uargs.count() == 3 || uargs.count() == 4)
            qci->addWidget(wa, uargs.at(1).toInt(), uargs.at(2).toInt(), f);
    } else if (method == "addLayout") {
        void *va = uargs.at(0).value<void*>();            
        QLayout *wa = (QLayout*)va;
        if (uargs.count() == 5 || uargs.count() == 6)
            qci->addLayout(wa, uargs.at(1).toInt(), uargs.at(2).toInt(),
                           uargs.at(3).toInt(), uargs.at(4).toInt(), f);
        if (uargs.count() == 3 || uargs.count() == 4)
            qci->addLayout(wa, uargs.at(1).toInt(), uargs.at(2).toInt(), f);
    } else {
        qDebug()<<"unsupported method:"<<method;
    }
    
    return QVariant();
}

