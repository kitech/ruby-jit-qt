#include "fix_clang_undef_eai.h"

/////////
#include <llvm/Support/Host.h>

#include <clang/AST/AST.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/APValue.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"


#include "invokestorage.h"
#include "frontengine.h"

FrontEngine::FrontEngine()
{
    this->initCompiler();
}

FrontEngine::~FrontEngine()
{
}


bool FrontEngine::init()
{
    
    return true;
}

bool FrontEngine::initCompiler()
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
        (char*)"-I/usr/include/qt/QtGui", (char*)"-I/usr/include/qt/QtWidgets",
        (char*)"-I/usr/include/qt/QtNetwork",
        (char*)"-I/usr/lib/clang/3.5.0/include",
    };
    static int argc = 11;
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

bool FrontEngine::parseClass(QString klass)
{
    QString fname = QString("%1.h").arg(klass).toLower();
    QString bdir = "/usr/include/qt";
    QVector<QString> cats = {"QtCore", "QtWidgets", "QtGui", "QtNetwork"};
    QString cat;
    
    if (units.contains(fname)) {
        qDebug()<<"already parsed, omit."<<fname;
        return true;
    }

    QString path;
    for (int i = 0; i < cats.count(); i++) {
        QString fpath = QString("%1/%2/%3").arg(bdir).arg(cats.at(i)).arg(fname);
        if (QFile::exists(fpath)) {
            cat = cats.at(i);
            path = fpath;
            break;
        }
    }

    if (path.isEmpty()) {
        qDebug()<<"class not found, no parse:"<<klass;
    } 
    return parseHeader(path);
    return true;
}

clang::ASTContext &FrontEngine::getASTContext()
{
    return mrgunit->getASTContext();
    // return mtrunit->getASTContext();
}

bool FrontEngine::loadPreparedASTFile()
{
    // 可以看作是parseHeader方法的正式版本，实时方法，不过一般通过自身内部调度调用

    if (mtrunit != NULL) {
        qDebug()<<"ast file alread loaded.";
        return true;
    }

    std::string astfile = "data/qthdrsrc.ast";
    clang::IntrusiveRefCntPtr<clang::DiagnosticsEngine> mydiag;
    clang::FileSystemOptions fsopts;

    QDateTime btime = QDateTime::currentDateTime();
    clang::ASTUnit *unit = clang::ASTUnit::LoadFromASTFile(astfile, mydiag, fsopts);
        // clang::ASTUnit::LoadFromASTFile(const std::string &Filename, 
        //                                 IntrusiveRefCntPtr<clang::DiagnosticsEngine> Diags, 
        //                                 const clang::FileSystemOptions &FileSystemOpts, 
        //                                 bool OnlyLocalDecls, 
        //                                 ArrayRef<RemappedFile> RemappedFiles);
    QDateTime etime = QDateTime::currentDateTime();
    if (!unit) qFatal("load ast faild.");
    
    clang::ASTContext &tctx = unit->getASTContext();
    clang::SourceManager &srcman = unit->getSourceManager();
    clang::FileManager &fman = unit->getFileManager();
    clang::TranslationUnitDecl *trud = tctx.getTranslationUnitDecl();

    if (trud == NULL) {
        qDebug()<<"load ast file error.";
        return false;
    }

    qDebug()<<"has sema:"<<unit->hasSema();

    mrgunit = unit;
    mtrunit = trud;
    mgctx = tctx.createMangleContext();

    int ndic = std::count_if(trud->decls_begin(), trud->decls_end(), [](clang::Decl *d){return true;});
    int dic = 0;
    for (auto it = trud->decls_begin(); it != trud->decls_end(); it++) {
        dic++;
    }
    qDebug()<<trud<<trud->decls_empty()<<"decls count:"<<dic<<ndic;

    // 遍历一次用时120ms,加载5,6ms，很快了。3M内存？？？
    qDebug()<<"time "<<etime.msecsTo(btime)
            <<tctx.getASTAllocatedMemory()
            <<tctx.getSideTableAllocatedMemory();

    return true;
}

void FrontEngine::dumpast()
{
    clang::ASTUnit *unit = mrgunit;
    clang::ASTContext &tctx = unit->getASTContext();
    clang::SourceManager &srcman = unit->getSourceManager();
    clang::FileManager &fman = unit->getFileManager();
    clang::TranslationUnitDecl *trud = tctx.getTranslationUnitDecl();

    qDebug()<<"==========";
    tctx.PrintStats();
    qDebug()<<"==========";
    srcman.PrintStats();
    qDebug()<<"==========";
    fman.PrintStats();
    qDebug()<<"==========";

    int num = std::count_if(tctx.local_imports().begin(),
                            tctx.local_imports().end(), [](clang::ImportDecl *d) {return true;});
    qDebug()<<"importD:"<<num;
    num = std::count_if(trud->decls_begin(), trud->decls_end(), [](clang::Decl *d){return true;});
    qDebug()<<"decls:"<<num;
    clang::ExternalASTSource *extsrc = tctx.getExternalSource();
    qDebug()<<"extsrc:"<<extsrc<<extsrc->getModule(0); // 0x8322, 0
    // extsrc->PrintStats();
    
    qDebug()<<"toplevel:"<<unit->top_level_size()<<unit->top_level_empty();
    // 
    auto fety = fman.getFile("/usrqstring.h");
    qDebug()<<"fety:"<<fety; // 0
    
    // 
    num = std::count_if(srcman.fileinfo_begin(), srcman.fileinfo_end(),
                        [](std::pair<const clang::FileEntry*, clang::SrcMgr::ContentCache*> f){return true;});
    qDebug()<<"fileinfo:"<<num<<srcman.getPreambleFileID().isInvalid(); // 0

    qDebug()<<"oris:"<<unit->getOriginalSourceFileName().data();
    qDebug()<<fman.getFile(unit->getOriginalSourceFileName());
    auto fety2 = fman.getFile(unit->getOriginalSourceFileName());
    
    qDebug()<<"from ast:"<<trud->isFromASTFile()<<"own mod:"<<trud->getOwningModuleID();
}

// from ast file
bool FrontEngine::parseHeader()
{
    std::string astfile = "data/qthdrsrc.ast";
    clang::IntrusiveRefCntPtr<clang::DiagnosticsEngine> mydiag;
    clang::FileSystemOptions fsopts;

    QDateTime btime = QDateTime::currentDateTime();
    clang::ASTUnit *unit = clang::ASTUnit::LoadFromASTFile(astfile, mydiag, fsopts);
        // clang::ASTUnit::LoadFromASTFile(const std::string &Filename, 
        //                                 IntrusiveRefCntPtr<clang::DiagnosticsEngine> Diags, 
        //                                 const clang::FileSystemOptions &FileSystemOpts, 
        //                                 bool OnlyLocalDecls, 
        //                                 ArrayRef<RemappedFile> RemappedFiles);
    QDateTime etime = QDateTime::currentDateTime();
    clang::ASTContext &tctx = unit->getASTContext();
    clang::SourceManager &srcman = unit->getSourceManager();
    clang::FileManager &fman = unit->getFileManager();
    clang::TranslationUnitDecl *trud = tctx.getTranslationUnitDecl();
    // bool bret = unit->Reparse();
    // unit->Save("abc.ast");
    tctx.PrintStats();
    srcman.PrintStats();
    fman.PrintStats();

    unit->getStartOfMainFileID();
    
    // unit->loadModule(unit->getStartOfMainFileID());

    int fic = 0;
    mgctx = tctx.createMangleContext();

    for (auto it = srcman.fileinfo_begin(); it != srcman.fileinfo_end(); it++) {
        fic ++;
        const clang::FileEntry *fe = it->first;
        qDebug()<<it->first<<it->second<<fe->getName()<<fe->getSize();
    }
    int dic = 0;
    for (auto it = trud->decls_begin(); it != trud->decls_end(); it++) {
        dic++;
        clang::Decl *d = *it;
        if (dic % 100 == 1) {
            qDebug()<<d->getDeclKindName();
            if (QString(d->getDeclKindName()) == "CXXMethod") {
                clang::CXXMethodDecl *mthdecl = llvm::cast<clang::CXXMethodDecl>(d);
                std::string str;
                llvm::raw_string_ostream los(str);
                mgctx->mangleCXXName(mthdecl, los);
                qDebug()<<los.str().c_str();
            }
        }
    }
    qDebug()<<"fic:"<<fic<<trud<<trud->decls_empty()<<"decls count:"<<dic;

    // 遍历一次用时120ms,加载5,6ms，很快了。3M内存？？？
    qDebug()<<"time "<<etime.msecsTo(btime)
            <<tctx.getASTAllocatedMemory()
            <<tctx.getSideTableAllocatedMemory();

    qDebug()<<unit<<unit->isMainFileAST()<<unit->getOriginalSourceFileName().data()
            <<unit->getASTFileName().data()
            <<unit->getMainFileName().data()
            <<unit->top_level_size()
            <<unit->top_level_empty()
            <<unit->isModuleFile()
            <<unit->getPCHFile()
        ;


    return true;
}

bool FrontEngine::parseHeader(QString path)
{
    qDebug()<<path;

    // path = "./qthdrsrc.h";
    QFile fp(path);
    fp.open(QIODevice::ReadOnly);
    QByteArray ba = fp.readAll();
    fp.close();

    const char *pcode = strdup(ba.data());
    llvm::MemoryBuffer *mbuf = llvm::MemoryBuffer::getMemBuffer(pcode);
    clang::PreprocessorOptions &ppopt = mcis->getPreprocessorOpts();
    ppopt.addRemappedFile("flycode.cxx", mbuf);

    QDateTime btime = QDateTime::currentDateTime();
    clang::IntrusiveRefCntPtr<clang::DiagnosticsEngine> mydiag;
    //        clang::CompilerInstance::createDiagnostics(mcis->
    std::unique_ptr<clang::ASTUnit> pastu = 
        clang::ASTUnit::LoadFromCompilerInvocation(mciv, mydiag);
    clang::ASTContext &tctx = pastu->getASTContext();
    QDateTime etime = QDateTime::currentDateTime();

    // pastu->Save("abc.ast");

    // about 3.5M for qstring.h, 310ms
    // about 9.3M for qwidget.h, 770ms
    // about 8.8M for qapplication.h, 700ms
    // 复杂点的可能都会接近10M，700ms
    qDebug()<<"time "<<etime.msecsTo(btime)
            <<tctx.getASTAllocatedMemory()
            <<tctx.getSideTableAllocatedMemory();

    qDebug()<<pastu.get()<<pastu->isMainFileAST()<<pastu->getOriginalSourceFileName().data()
            <<pastu->top_level_size();

    clang::ASTUnit *unit = pastu.release();
    clang::ASTUnit *astu = unit;
    // should be qxxx.h
    units.insert(*(--path.split("/").end()), unit);

    for (auto it = astu->top_level_begin(); it != astu->top_level_end(); it++) {
        clang::Decl *decl = *it;
        clang::CXXRecordDecl *recdecl;
        clang::CXXMethodDecl *mthdecl;
        QString declname;
        // qDebug()<<decl<<decl->getDeclKindName()<<decl->getKind();
        switch (decl->getKind()) {
        case clang::Decl::CXXMethod:
            break;
        case clang::Decl::CXXRecord:
            recdecl = llvm::cast<clang::CXXRecordDecl>(decl);
            // qDebug()<<"name.."<<recdecl->getName().data();
            declname = QString(recdecl->getName().data());
            if (declname == "QString") {
                // decl->dump();
            }
            break;
        default:
            break;
        }
    }

    // clean up memory
    // delete pcode;
    
    return true;
}

#include <cxxabi.h>
bool FrontEngine::get_method_default_args(QString klass, QString method, QString symbol_name
                                         , QVector<QVariant> &dargs)
{

    QString file = QString("%1.h").arg(klass).toLower();
    qDebug()<<units.keys()<<file;
    if (!units.contains(file)) {
        return false;
    }

    clang::ASTUnit *unit = units.value(file);
    qDebug()<<klass<<unit;

    // 1st, find class record decl
    // 2nd, find method decl
    // 2.5nd, overload method resolve
    // 3rd, find the default arg's value
    clang::CXXRecordDecl * res_recdecl = NULL;
    clang::CXXMethodDecl * res_mthdecl = NULL;

    for (auto it = unit->top_level_begin(); it != unit->top_level_end(); it++) {
        clang::Decl *decl = *it;
        clang::CXXRecordDecl *recdecl;
        clang::CXXMethodDecl *mthdecl;
        QString declname;
        // qDebug()<<decl<<decl->getDeclKindName()<<decl->getKind();
        switch (decl->getKind()) {
        case clang::Decl::CXXMethod:
            break;
        case clang::Decl::CXXRecord:
            recdecl = llvm::cast<clang::CXXRecordDecl>(decl);
            // qDebug()<<"name.."<<recdecl->getName().data();
            declname = QString(recdecl->getName().data());
            if (recdecl->hasDefinition() && declname == klass) {
                res_recdecl = recdecl;
                break;
            }
            break;
        default:
            break;
        }

    }

    // trim prefix class
    auto trim_type_class = [] (QString type_str) -> QString {
        QStringList type_elems = type_str.split(" ");
        for (int i = 0; i < type_elems.count(); i ++) {
            if (type_elems.at(i) == "class") {
                type_elems.removeAt(i);
            }
        }

        return type_elems.join(" ");
    };

    auto demangle = [](QString mname) -> QString {
        size_t dmlen = 128*2;
        char *dmname = (char*)calloc(1, dmlen);
        int dmstatus = -1;
        __cxxabiv1::__cxa_demangle(mname.toLatin1().data(), dmname, &dmlen, &dmstatus);
        qDebug()<<mname<<dmname<<dmlen<<dmstatus;
        if (dmstatus == 0) {
            return QString(dmname);
        }
        qDebug()<<"error:"<<strerror(errno);
        return QString();
    };

    if (res_recdecl) {
        clang::CXXRecordDecl *recdecl = res_recdecl;

        QString dmname = demangle(symbol_name);

        for (auto ait = recdecl->method_begin(); ait != recdecl->method_end(); ait++) {
            clang::CXXMethodDecl *mthdecl = *ait;
            // qDebug()<<"name .."<<mthdecl->getName().data();
            if (QString(mthdecl->getName().data()) == method) {
                qDebug()<<"found it, maybe rc:";
                mthdecl->dump();
                
                QString tmp_dmname = QString("Ya%1::%2(").arg(klass).arg(method);
                for (auto bit = mthdecl->param_begin(); bit != mthdecl->param_end(); bit++) {
                    int idx = bit - mthdecl->param_begin();
                    clang::ParmVarDecl *pvdecl = *bit;
                    clang::QualType pvtype = pvdecl->getType();
                    qDebug()<<pvtype.getAsString().c_str();
                    tmp_dmname += QString("%1%2")
                        .arg(trim_type_class(QString(pvtype.getAsString().c_str())))
                        .arg(idx == mthdecl->param_size() - 1 ? "" : ", ");
                }
                tmp_dmname += QString(")%1").arg(mthdecl->isConst() ? " const" : "");

                QString s1 = QMetaObject::normalizedSignature(dmname.toLatin1().data());
                QString s2 = QMetaObject::normalizedSignature(tmp_dmname.toLatin1().data());
                qDebug()<<"tmp name:"<<tmp_dmname<<"==?"<<(tmp_dmname == dmname)
                        <<(s1 == s2);

                if (s1 == s2) {
                    res_mthdecl = mthdecl;
                    break;
                }
            }
        }

    }

    auto dyn_create_obj = [](clang::ParmVarDecl *decl, clang::CXXConstructExpr *expr) -> QVariant {
        clang::QualType type = decl->getType();
        QString cname = type.getAsString().data();
        clang::Expr *rexpr = 0; // expr->getResultExpr();
        // qDebug()<<cname<<expr->getNumArgs()<<rexpr<<(rexpr?rexpr->getStmtClassName():"");
        if (cname == "class QChar") {
            // TODO more work
            return QChar(' ');
        }

        return 1763;
    };

    // QVector<QVariant> dargs;
    if (res_mthdecl) {
        clang::CXXMethodDecl *mthdecl = res_mthdecl;
        dargs.resize(mthdecl->param_size());
        for (auto bit = mthdecl->param_begin(); bit != mthdecl->param_end(); bit ++) {
            int idx = bit - mthdecl->param_begin();
            clang::ParmVarDecl *pvdecl = *bit;
            qDebug()<<"param:"<<pvdecl->getName().data()<<pvdecl->hasDefaultArg()
                    <<pvdecl->getDefaultArg()
                    <<(bit - mthdecl->param_begin());
            clang::Expr *daexpr = pvdecl->getDefaultArg();
            if (daexpr) {
                qDebug()<<"expr:"<<daexpr->getValueKind()
                        <<daexpr->isIntegerConstantExpr(unit->getASTContext())
                        <<daexpr->getStmtClassName();
                // int
                llvm::APSInt daiv;
                bool bret = daexpr->isIntegerConstantExpr(daiv, unit->getASTContext());
                qDebug()<<"valid int v:"<<bret<<daiv.getZExtValue();
                if (bret) {
                    dargs[idx] = QVariant((qlonglong)daiv.getZExtValue());
                }

                // CXXContructExpr
                // qDebug()<<clang::isa<clang::CXXConstructExpr>(daexpr);
                if (clang::isa<clang::CXXConstructExpr>(daexpr)) {
                    QVariant v = dyn_create_obj(pvdecl, clang::cast<clang::CXXConstructExpr>(daexpr));
                    dargs[idx] = v;
                }
            }
        }

    }

    qDebug()<<dargs;

    if (res_recdecl && res_mthdecl) {
        return true;
    }

    return false;
    return true;
}

bool FrontEngine::get_method_default_args2(QString klass, QString method, QString symbol_name
                                         , QVector<QVariant> &dargs)
{
    this->loadPreparedASTFile();

    QString yaklass = QString("Ya%1").arg(klass);
    QString yamethod = method;
    if (klass == method) yamethod = yaklass;
    clang::CXXRecordDecl *recdecl = this->find_class_decl(yaklass);
    QVector<clang::CXXMethodDecl*> mthdecls = this->find_method_decls(recdecl, yaklass, yamethod);

    clang::CXXMethodDecl *mthdecl = NULL;
    for (clang::CXXMethodDecl *d: mthdecls) {
        QString tmp_symbol;
        QString tmp_prototype;
        bool ok = this->mangle_method_to_symbol(d, tmp_symbol, tmp_prototype);
        if (ok && tmp_symbol == symbol_name) {
            mthdecl = d;
            break;
        }
    }
    qDebug()<<"mat:"<<mthdecl;

    if (mthdecl == NULL) {
        return false;
    }

    bool ok = this->get_method_default_params(mthdecl, dargs);
    Q_ASSERT(ok);
    
    return ok;
    return false;
}

bool FrontEngine::resolve_symbol(QString klass, QString method, QVector<QVariant> uargs,
                    QString &symbol_name, QString &proto_str)
{
    this->loadPreparedASTFile();

    QString yaklass = QString("Ya%1").arg(klass);
    QString yamethod = method;
    if (klass == method) yamethod = yaklass;
    clang::CXXRecordDecl *recdecl = this->find_class_decl(yaklass);
    QVector<clang::CXXMethodDecl*> mthdecls = this->find_method_decls(recdecl, yaklass, yamethod);
    bool match = false;

    QVector<clang::CXXMethodDecl*> mats;
    for (clang::CXXMethodDecl *d: mthdecls) {
        match = this->method_match_by_uargs(d, yaklass, yamethod, uargs);
        if (match) {
            mats << d;
        }
    }
    qDebug()<<"matcc:"<<mats.count()<<"mats:"<<mats;
    
    if (mats.count() <= 0) {
        qDebug()<<"method not found:";
        return false;
    }

    if (mats.count() > 1) {
        qDebug()<<"find more matched method, try first now.";
    }

    clang::CXXMethodDecl *md = mats.at(0);
    bool ok = this->mangle_method_to_symbol(md, symbol_name, proto_str);
    Q_ASSERT(ok);
    return ok;
    return true;
}

bool FrontEngine::get_method_return_type(QString klass, QString method, QVector<QVariant> uargs, 
                                         QString symbol_name, QVariant &retype)
{
    this->loadPreparedASTFile();
    
    QString yaklass = QString("Ya%1").arg(klass);
    QString yamethod = method;
    if (klass == method) yamethod = yaklass;
    clang::CXXRecordDecl *recdecl = this->find_class_decl(yaklass);
    QVector<clang::CXXMethodDecl*> mthdecls = this->find_method_decls(recdecl, yaklass, yamethod);

    clang::CXXMethodDecl *mthdecl = NULL;
    for (clang::CXXMethodDecl *d: mthdecls) {
        QString tmp_symbol;
        QString tmp_prototype;
        d->dumpColor();
        bool ok = this->mangle_method_to_symbol(d, tmp_symbol, tmp_prototype);
        if (ok && tmp_symbol == symbol_name) {
            mthdecl = d;
            break;
        }
    }
    qDebug()<<"mat:"<<mthdecl;

    if (mthdecl == NULL) {
        return false;
    }

    QVariant tmp_retype = this->get_method_return_type(mthdecl);
    retype = tmp_retype;

    return retype.isValid();
    return false;
}


///// privates
clang::FunctionDecl *FrontEngine::find_free_function(QString fname)
{
    clang::TranslationUnitDecl *udecl = this->mtrunit;
    qDebug()<<fname<<udecl;
    Q_ASSERT(udecl != NULL);

    clang::FunctionDecl *res_fundecl = NULL;
    for (auto it = udecl->decls_begin(); it != udecl->decls_end(); it++) {
        clang::Decl *decl = *it;
        clang::FunctionDecl *fundecl;
        QString declname;
        // qDebug()<<decl<<decl->getDeclKindName()<<decl->getKind();
        switch (decl->getKind()) {
        case clang::Decl::Function:
            fundecl = llvm::cast<clang::FunctionDecl>(decl);
            declname = QString(fundecl->getName().data());
            if (declname == fname) {
                res_fundecl = fundecl;
            }
            break;
        default:
            break;
        }

    }
    
    if (res_fundecl == NULL) {
        qDebug()<<"func decl not found:"<<fname;
        return NULL;
    }
    return res_fundecl;
}

clang::FunctionDecl *FrontEngine::find_free_function2(QString symname)
{
    clang::TranslationUnitDecl *udecl = this->mtrunit;
    qDebug()<<symname<<udecl;
    Q_ASSERT(udecl != NULL);

    clang::FunctionDecl *res_fundecl = NULL;
    for (auto it = udecl->decls_begin(); it != udecl->decls_end(); it++) {
        clang::Decl *decl = *it;
        clang::FunctionDecl *fundecl;
        QString declname;
        QString proto_str;
        // qDebug()<<decl<<decl->getDeclKindName()<<decl->getKind();
        switch (decl->getKind()) {
        case clang::Decl::Function:
            fundecl = llvm::cast<clang::FunctionDecl>(decl);
            mangle_function_to_symbol(fundecl, declname, proto_str);
            if (declname.startsWith("_Zls6QDebugRK10")) 
                qDebug()<<declname<<symname;
            if (declname == symname) {
                res_fundecl = fundecl;
                break;
            }
            break;
        default:
            break;
        }

    }
    
    if (res_fundecl == NULL) {
        qDebug()<<"func decl not found:"<<symname;
        return NULL;
    }
    return res_fundecl;
}

clang::CXXRecordDecl* FrontEngine::find_class_decl(QString klass)
{
    clang::TranslationUnitDecl *udecl = this->mtrunit;
    qDebug()<<klass<<udecl;
    Q_ASSERT(udecl != NULL);

    // 1st, find class record decl
    // 2nd, find method decl
    // 2.5nd, overload method resolve
    // 3rd, find the default arg's value
    clang::CXXRecordDecl * res_recdecl = NULL;
    clang::CXXMethodDecl * res_mthdecl = NULL;

    for (auto it = udecl->decls_begin(); it != udecl->decls_end(); it++) {
        clang::Decl *decl = *it;
        clang::CXXRecordDecl *recdecl;
        clang::CXXMethodDecl *mthdecl;
        QString declname;
        // qDebug()<<decl<<decl->getDeclKindName()<<decl->getKind();
        switch (decl->getKind()) {
        case clang::Decl::CXXMethod:
            break;
        case clang::Decl::CXXRecord:
            recdecl = llvm::cast<clang::CXXRecordDecl>(decl);
            declname = QString(recdecl->getName().data());
            // qDebug()<<"name.."<<recdecl->getName().data()<<recdecl->hasBody();
            if (recdecl->isCompleteDefinition() && declname == klass) {
                qDebug()<<"name.."<<recdecl->getName().data()<<recdecl->hasBody()
                        <<recdecl->isBeingDefined()
                        <<recdecl->isCanonicalDecl()<<recdecl->isFirstDecl()
                        <<recdecl->isCompleteDefinition();
                res_recdecl = recdecl;
                // recdecl->dump();
                break;
            }
            break;
            // case clang::Decl::ClassTemplate:    break;
        default:
            /*
            if (clang::isa<clang::NamedDecl>(decl)) {
                clang::NamedDecl *nd = clang::cast<clang::NamedDecl>(decl);
                declname = QString(nd->getName().data());
                if (declname.indexOf("QTypedArrayData") >= 0) {
                    nd->dumpColor();
                    exit(0);
                }
            }
            */
            break;
        }

    }
    
    if (res_recdecl == NULL) {
        qDebug()<<"klass decl not found:";
        return NULL;
    }

    clang::CXXRecordDecl *recdecl = res_recdecl;
    return recdecl;
}



clang::ClassTemplateDecl* FrontEngine::find_tpl_class_decl(QString klass)
{
    clang::TranslationUnitDecl *udecl = this->mtrunit;
    qDebug()<<klass<<udecl;
    Q_ASSERT(udecl != NULL);

    // 1st, find class record decl
    // 2nd, find method decl
    // 2.5nd, overload method resolve
    // 3rd, find the default arg's value
    clang::ClassTemplateDecl *res_tpldecl = NULL;

    for (auto it = udecl->decls_begin(); it != udecl->decls_end(); it++) {
        clang::Decl *decl = *it;
        clang::ClassTemplateDecl *tpldecl;
        QString declname;
        // qDebug()<<decl<<decl->getDeclKindName()<<decl->getKind();
        switch (decl->getKind()) {
        case clang::Decl::ClassTemplate:
            tpldecl = llvm::cast<clang::ClassTemplateDecl>(decl);
            declname = QString(tpldecl->getName().data());
            // qDebug()<<"name.."<<recdecl->getName().data()<<recdecl->hasBody();
            if (declname == klass) {
                res_tpldecl = tpldecl;
                // recdecl->dump();
            }
            break;
        default:
            /*
            if (clang::isa<clang::NamedDecl>(decl)) {
                clang::NamedDecl *nd = clang::cast<clang::NamedDecl>(decl);
                declname = QString(nd->getName().data());
                if (declname.indexOf("QTypedArrayData") >= 0) {
                    nd->dumpColor();
                    exit(0);
                }
            }
            */
            break;
        }

    }
    
    if (res_tpldecl == NULL) {
        qDebug()<<"tpl klass decl not found:";
        return NULL;
    }

    clang::ClassTemplateDecl *tpldecl = res_tpldecl;
    return tpldecl;
}

QVector<clang::CXXMethodDecl*>
FrontEngine::find_method_decls(clang::CXXRecordDecl *decl, QString klass, QString method)
{
    QVector<clang::CXXMethodDecl*> mdecls;

    clang::CXXRecordDecl *recdecl = decl;

    for (auto ait = recdecl->method_begin(); ait != recdecl->method_end(); ait++) {
        clang::CXXMethodDecl *mthdecl = *ait;
        // qDebug()<<"name .."<<mthdecl->getName().data();
        if (QString(mthdecl->getName().data()).length() == 0) {
            // mthdecl->dumpColor();
        }
        if (klass == method && clang::isa<clang::CXXConstructorDecl>(mthdecl)) {
            mdecls.append(mthdecl);
            // mthdecl->dumpColor();
        } else if (QString(mthdecl->getName().data()) == method) {
            qDebug()<<"found it, maybe rc:"<<method;
            // mthdecl->dumpColor();
            mdecls.append(mthdecl);
                
            // QString tmp_dmname = QString("Ya%1::%2(").arg(klass).arg(method);
            // for (auto bit = mthdecl->param_begin(); bit != mthdecl->param_end(); bit++) {
            //     int idx = bit - mthdecl->param_begin();
            //     clang::ParmVarDecl *pvdecl = *bit;
            //     clang::QualType pvtype = pvdecl->getType();
            //     qDebug()<<pvtype.getAsString().c_str();
            //     tmp_dmname += QString("%1%2")
            //         .arg(trim_type_class(QString(pvtype.getAsString().c_str())))
            //         .arg(idx == mthdecl->param_size() - 1 ? "" : ", ");
            // }
            // tmp_dmname += QString(")%1").arg(mthdecl->isConst() ? " const" : "");

            // QString s1 = QMetaObject::normalizedSignature(dmname.toLatin1().data());
            // QString s2 = QMetaObject::normalizedSignature(tmp_dmname.toLatin1().data());
            // qDebug()<<"tmp name:"<<tmp_dmname<<"==?"<<(tmp_dmname == dmname)
            //         <<(s1 == s2);

            // if (s1 == s2) {
            //     res_mthdecl = mthdecl;
            //     break;
            // }
        }
    }
    qDebug()<<"found method count:"<<method<<mdecls.count();
    
    return mdecls;
}

QVector<clang::CXXMethodDecl*>
FrontEngine::find_tpl_method_decls(clang::ClassTemplateDecl *decl, QString klass, QString method)
{
    QVector<clang::CXXMethodDecl*> mdecls;

    clang::CXXRecordDecl *recdecl = decl->getTemplatedDecl();

    for (auto ait = recdecl->method_begin(); ait != recdecl->method_end(); ait++) {
        clang::CXXMethodDecl *mthdecl = *ait;
        // qDebug()<<"name .."<<mthdecl->getName().data();
        if (QString(mthdecl->getName().data()).length() == 0) {
            // mthdecl->dumpColor();
        }
        if (klass == method && clang::isa<clang::CXXConstructorDecl>(mthdecl)) {
            mdecls.append(mthdecl);
            // mthdecl->dumpColor();
        } else if (QString(mthdecl->getName().data()) == method) {
            qDebug()<<"found it, maybe rc:"<<method;
            // mthdecl->dumpColor();
            mdecls.append(mthdecl);
                
            // QString tmp_dmname = QString("Ya%1::%2(").arg(klass).arg(method);
            // for (auto bit = mthdecl->param_begin(); bit != mthdecl->param_end(); bit++) {
            //     int idx = bit - mthdecl->param_begin();
            //     clang::ParmVarDecl *pvdecl = *bit;
            //     clang::QualType pvtype = pvdecl->getType();
            //     qDebug()<<pvtype.getAsString().c_str();
            //     tmp_dmname += QString("%1%2")
            //         .arg(trim_type_class(QString(pvtype.getAsString().c_str())))
            //         .arg(idx == mthdecl->param_size() - 1 ? "" : ", ");
            // }
            // tmp_dmname += QString(")%1").arg(mthdecl->isConst() ? " const" : "");

            // QString s1 = QMetaObject::normalizedSignature(dmname.toLatin1().data());
            // QString s2 = QMetaObject::normalizedSignature(tmp_dmname.toLatin1().data());
            // qDebug()<<"tmp name:"<<tmp_dmname<<"==?"<<(tmp_dmname == dmname)
            //         <<(s1 == s2);

            // if (s1 == s2) {
            //     res_mthdecl = mthdecl;
            //     break;
            // }
        }
    }
    qDebug()<<"found method count:"<<method<<mdecls.count();
    
    return mdecls;
}

// template<typename DT>
QVector<clang::CXXConstructorDecl*>
resolve_callee_decl(QVector<clang::CXXConstructorDecl*> &decls, QVector<QVariant> &uargs)
{
    QVector<clang::CXXConstructorDecl*> res;
    if (uargs.count() > 0) {
        // drop default ctor
        for (auto d: decls) {
            if (!d->isDefaulted()) {
                res.append(d);
            }
        }
    }
    if (res.count() <= 1) {
        return res;
    }

    // next step, 定义的参数个数>=调用提供的参数个数
    QVector<clang::CXXConstructorDecl*> res2;
    QVector<int> score;
    for (auto d: res2) score.append(1);

    int idx = -1;
    for (auto d: res) {
        idx ++;
        int callee_arg_num = d->getNumParams();
        if (callee_arg_num >= uargs.count()) {
            res2.append(d);
        }
    }

    if (res.count() <= 1) return res2;
    return res2;

    QVector<clang::CXXConstructorDecl*> resn;
    return resn;
}

template<typename DT>
QVector<DT*> resolve_callee_decl(QVector<DT*> &decls, QVector<QVariant> &uargs)
{
    QVector<DT*> res;
    if (uargs.count() > 0) {
        // drop default ctor
        for (auto d: decls) {
            if (!d->isDefaulted()) {
                res.append(d);
            }
        }
    }
    if (res.count() <= 1) {
        return res;
    }

    // next step, 定义的参数个数>=调用提供的参数个数
    QVector<DT*> res2;
    QVector<int> score;
    for (auto d: res2) score.append(1);

    int idx = -1;
    for (auto d: res) {
        idx ++;
        int callee_arg_num = d->getNumParams();
        if (callee_arg_num >= uargs.count()) {
            res2.append(d);
        }
    }

    if (res.count() <= 1) return res2;
    return res2;

    QVector<DT*> resn;
    return resn;
}

clang::CXXConstructorDecl*
FrontEngine::find_ctor_decl(clang::CXXRecordDecl *decl, 
                            QString klass, QVector<QVariant> uargs)
{
    bool match = false;
    QVector<clang::CXXConstructorDecl*> ctors;
    for (auto d: decl->methods()) {
        if (!clang::isa<clang::CXXConstructorDecl>(d)) continue;
        auto cd = clang::cast<clang::CXXConstructorDecl>(d);
        // if (!cd->isDefaultConstructor()) continue; // 目前先使用默认构造函数
        // return cd;
        match = this->method_match_by_uargs(cd, klass, klass, uargs);
        if (match) {
            ctors.append(cd);
        }
    }
    qDebug()<<"matcc:"<<ctors.count()<<"mats:"<<ctors;

    if (ctors.count() <= 0) {
        qDebug()<<"method not found:";
        // return this->find_method_decl_from_base(decl, klass, method, uargs);
        return NULL;
    }
    else if (ctors.count() > 1) {
        qDebug()<<"find more matched method, try first now."<<ctors.count();
        auto rcs = resolve_callee_decl(ctors, uargs);
        ctors = rcs;
        qDebug()<<rcs.count()<<rcs;
    }
    // else ctors.count() == 1

    clang::CXXConstructorDecl *ctor = ctors.at(0);
    Q_ASSERT(md);
    return ctor;
}

// 查找一个类的符合条件的方法定义。
clang::CXXMethodDecl* FrontEngine::find_method_decl(clang::CXXRecordDecl *decl, 
                                       QString klass, QString method, QVector<QVariant> uargs)
{
    this->loadPreparedASTFile();

    clang::CXXRecordDecl *recdecl = this->find_class_decl(klass);
    QVector<clang::CXXMethodDecl*> mthdecls = this->find_method_decls(recdecl, klass, method);
    bool match = false;

    QVector<clang::CXXMethodDecl*> mats;
    for (clang::CXXMethodDecl *d: mthdecls) {
        match = this->method_match_by_uargs(d, klass, method, uargs);
        if (match) {
            mats << d;
        }
    }
    qDebug()<<"matcc:"<<mats.count()<<"mats:"<<mats;
    
    if (mats.count() <= 0) {
        qDebug()<<"method not found:"<<method;
        return this->find_method_decl_from_base(decl, klass, method, uargs);
        return NULL;
    }
    else if (mats.count() > 1) {
        qDebug()<<"find more matched method, try first now."<<mats.count();
        // auto rcs = resolve_callee_decl<clang::CXXMethodDecl>(mats, uargs);
        // mats = rcs;
        // qDebug()<<rcs.count()<<rcs;        
    }
    // else mats.count() == 1

    clang::CXXMethodDecl *md = mats.at(0);
    Q_ASSERT(md);
    return md;
}

clang::CXXMethodDecl* 
FrontEngine::find_method_decl_from_base(clang::CXXRecordDecl *decl, 
                                        QString klass, QString method, QVector<QVariant> uargs)
{
    qDebug()<<"base classes:"<<decl->getNumBases();
    for (auto bs: decl->bases()) {
        auto bt = bs.getType();
        qDebug()<<bt.getAsString().data();
        auto tp = QString(bt.getAsString().data()).split(' ');
        qDebug()<<tp;
        QString bklass = tp.at(1);
        clang::CXXRecordDecl *bdecl = this->find_class_decl(bklass);
        clang::CXXMethodDecl *mth_decl = this->find_method_decl(bdecl, bklass, method, uargs);
        if (mth_decl != NULL) {
            return mth_decl;
        }
    }
    return 0;
}

// 查找一个类的符合条件的方法定义。
clang::CXXMethodDecl*
FrontEngine::find_static_method_decl(clang::CXXRecordDecl *decl, 
                                     QString klass, QString method, QVector<QVariant> uargs)
{
    this->loadPreparedASTFile();

    clang::CXXRecordDecl *recdecl = this->find_class_decl(klass);
    QVector<clang::CXXMethodDecl*> mthdecls = this->find_method_decls(recdecl, klass, method);
    bool match = false;

    QVector<clang::CXXMethodDecl*> mats;
    for (clang::CXXMethodDecl *d: mthdecls) {
        if (!d->isStatic()) continue;
        match = this->method_match_by_uargs(d, klass, method, uargs);
        if (match) {
            mats << d;
        }
    }
    qDebug()<<"matcc:"<<mats.count()<<"mats:"<<mats;
    
    if (mats.count() <= 0) {
        qDebug()<<"method not found:"<<method;
        return this->find_static_method_decl_from_base(decl, klass, method, uargs);
        return NULL;
    }
    else if (mats.count() > 1) {
        qDebug()<<"find more matched method, try first now.";
    }
    // else mats.count() == 1

    clang::CXXMethodDecl *md = mats.at(0);
    Q_ASSERT(md);
    return md;
}

clang::CXXMethodDecl* 
FrontEngine::find_static_method_decl_from_base(clang::CXXRecordDecl *decl, 
                                               QString klass, QString method, QVector<QVariant> uargs)
{
    qDebug()<<"base classes:"<<decl->getNumBases();
    for (auto bs: decl->bases()) {
        auto bt = bs.getType();
        qDebug()<<bt.getAsString().data();
        auto tp = QString(bt.getAsString().data()).split(' ');
        qDebug()<<tp;
        QString bklass = tp.at(1);
        clang::CXXRecordDecl *bdecl = this->find_class_decl(bklass);
        clang::CXXMethodDecl *mth_decl = this->find_static_method_decl(bdecl, bklass, method, uargs);
        if (mth_decl != NULL) {
            return mth_decl;
        }
    }
    return 0;
}

bool FrontEngine::method_match_by_uargs(clang::CXXMethodDecl *decl, 
                           QString klass, QString method, QVector<QVariant> uargs)
{
    /*
      解析过滤该方法的每一个参数，如果有给定的用户参数，则查看类型是否匹配
      如果类型不匹配，也无法隐式转换到方法需要的类型，则false，不匹配。
      否则，继续下一个参数。
      如果下一个参数没有提供用户参数，看是否有默认参数值，如果有，则匹配。
      如果没有，则记录累计缺失参数的个数，后续计算匹配度。
     */
    qDebug()<<uargs;
    int idx = 0;
    int pless = 0; //
    QVector<int> lessed;
    for (auto it = decl->param_begin(); it != decl->param_end(); it++, idx++) {
        clang::ParmVarDecl *pd = *it;
        clang::QualType ptype = pd->getType();
        clang::QualType nrptype = ptype.getNonReferenceType();
        QString tstr = ptype.getAsString().c_str(); // type str 

        qDebug()<<tstr;
        // qDebug()<<uargs.count()<<idx<<pd->hasDefaultArg()
        //         <<ptype->isIntegralOrEnumerationType()<<tstr
        //         <<ptype->isIntegerType()<<ptype->isIntegralType(mrgunit->getASTContext())
        //         <<nrptype->isIntegralOrEnumerationType()<<tstr
        //         <<nrptype->isIntegerType()<<nrptype->isIntegralType(mrgunit->getASTContext());
        if (idx < uargs.count()) {
            bool ok = false;
            switch ((int)uargs.at(idx).type()) {
            case QMetaType::Int: case QMetaType::UInt:
            case QMetaType::Long: case QMetaType::ULong:
            case QMetaType::LongLong: case QMetaType::ULongLong:
            case QMetaType::Short: case QMetaType::UShort:
                if (ptype->isIntegralOrEnumerationType()) ok = true;
                if (nrptype->isIntegralOrEnumerationType()) ok = true;
                // sth. like QWidget *p = 0
                if (uargs.at(idx).toInt() == 0 && ptype->isPointerType()) ok = true;
                break;
            case QMetaType::Double: case QMetaType::Float:
                if (ptype->isRealFloatingType()) ok = true;
                // qDebug()<<ptype->isRealFloatingType()<<ptype->isBuiltinType();
                break;
            case QMetaType::Bool:
                if (ptype->isBooleanType()) ok = true;
                break;
            case QMetaType::QString:
                if (tstr.indexOf("QString") != -1) ok = true;
                if (tstr.indexOf("char *") != -1) ok = true;
                break;
            case QMetaType::QChar:
                if (ptype->isCharType()) ok = true;
                break;
            case QMetaType::VoidStar:
                qDebug()<<ptype->isObjectType() << ptype->isPointerType()
                        <<ptype->isReferenceType();
                qDebug()<<nrptype->isObjectType() << nrptype->isPointerType()
                        <<nrptype->isReferenceType();
                if (ptype->isObjectType() && ptype->isPointerType()) ok = true;
                // for const QPalette& and similar syntax args
                if (ptype->isReferenceType() && nrptype->isObjectType()) ok = true;
                break;
            case QMetaType::QStringList:
                if (tstr.indexOf("char **") != -1) ok = true;
                if (tstr.indexOf("QStringList") != 01) ok = true;
                break;
            default: qDebug()<<"unknown type:"<<uargs.at(idx).type()<<uargs.at(idx)
                             <<ptype.getAsString().data()
                             <<ptype->isClassType()<<ptype->isPointerType()
                             <<ptype->isObjectType();
                break;
            }
            
            if (!ok) {
                pless ++; lessed << idx;
            }
        } else {
            if (!pd->hasDefaultArg()) {
                pless ++; lessed << idx;
                continue;
            }
            clang::Expr *dae = pd->getDefaultArg();
            // 在此不考虑其默认值了，只要确定有默认值就可以。
        }
    }

    double match_degree = idx == 0 ? 1.0 : (1.0*(idx - pless)/idx);
    qDebug()<<"param count:"<<idx<<"less count:"<<pless<<"lessed:"<<lessed
            <<"match degree:"<<match_degree;

    if (match_degree == 1.0 || pless == 0) return true;
    return false;
    return true;
}

bool FrontEngine::mangle_method_to_symbol(clang::CXXMethodDecl *decl, 
                             QString &symbol_name, QString &proto_str)
{
    Q_ASSERT(mgctx);

    std::string strc;
    llvm::raw_string_ostream stm(strc);
    mgctx->mangleCXXName(decl, stm); // TODO, change to mangleCXXThunk for class method
    // TODO mangle "operator new()" and "operator delete()"

    if (stm.str().length() > 0) {
        symbol_name = QString(stm.str().c_str());
        return true;
    }

    return false;
}

bool FrontEngine::mangle_function_to_symbol(clang::FunctionDecl *decl, 
                                            QString &symbol_name, QString &proto_str)
{
    Q_ASSERT(mgctx);

    std::string strc;
    llvm::raw_string_ostream stm(strc);
    mgctx->mangleCXXName(decl, stm); // TODO, change to mangleCXXThunk for class method
    // TODO mangle "operator new()" and "operator delete()"

    if (stm.str().length() > 0) {
        symbol_name = QString(stm.str().c_str());
        return true;
    }

    return false;
}


bool FrontEngine::get_method_default_params(clang::CXXMethodDecl *decl, QVector<QVariant> &dparams)
{
    QVector<QVariant> &dps = dparams;

    auto plain_cinit_value = [](clang::Expr *e, FrontEngine *tthis) -> QVariant {
        QStack<clang::Expr*> exps;
        exps.push(e);
        clang::Expr *re = NULL;
        while (!exps.isEmpty()) {
            clang::Expr *te = exps.pop();
            re = te;
            for (auto ce: te->children()) {
                if (llvm::isa<clang::Expr>(ce)) {
                    exps.push(llvm::cast<clang::Expr>(ce));
                }
            }
        }
        qDebug()<<re<<re->isEvaluatable(tthis->mtrunit->getASTContext());
        re->dumpColor();
        if (llvm::isa<clang::IntegerLiteral>(re)) {
            llvm::APInt v = llvm::cast<clang::IntegerLiteral>(re)->getValue();
            return QVariant::fromValue(v.getLimitedValue());
        }
        else if (llvm::isa<clang::CharacterLiteral>(re)) {
            unsigned char c = llvm::cast<clang::CharacterLiteral>(re)->getValue();
            return QVariant::fromValue(c);
        } else {
            qDebug()<<"unsupported expr:"<<re;
        }
        return QVariant();
    };

    auto eval_ctor = [&plain_cinit_value](clang::CXXConstructExpr* expr, FrontEngine *tthis) -> QVariant {
        clang::CXXConstructorDecl *decl = expr->getConstructor();
        clang::CXXRecordDecl *rec_decl = decl->getParent();
        QString klass_name = rec_decl->getName().data();
        decl->dumpColor();
        qDebug()<<"param klass name:"<<klass_name<<decl;
        if (klass_name == "QChar") {
            return QVariant(QChar(' '));
        } else if (klass_name == "QFlags") {
            qDebug()<<expr->isEvaluatable(tthis->mtrunit->getASTContext()); // false
            QVariant evlst = plain_cinit_value(expr, tthis);
            // if (evlst.isValid()) return evlst;

            QFlags<QUrl::ComponentFormattingOption> *f =
                new QFlags<QUrl::ComponentFormattingOption>(0);
            QFlags<Qt::WindowType> *fw =
                new QFlags<Qt::WindowType>(0);
            qDebug()<<(*f);
            QFlag f2(2);
            xQFlag f3 = {0};
            QVariant vf = QVariant::fromValue(f3);
            qDebug()<<vf<<vf.type()<<vf.userType();
            // QVariant pv = plain_cinit_value(expr, tthis);
            // qDebug()<<pv;
            // return vf;
            return QVariant::fromValue((void*)fw);
        }
        
        return QVariant(99813721);
    };

    int cnter = 0;
    qDebug()<<"============"<<dps.count();
    decl->dumpColor();
    for (auto it = decl->param_begin(); it != decl->param_end(); it++, cnter++) {
        clang::ParmVarDecl *pd = *it;
        qDebug()<<cnter<<dps;
        if (!pd->hasDefaultArg()) {          
            dps << QVariant();
            continue;
        }
        clang::Expr *dae = pd->getDefaultArg();
        clang::Expr *udae = pd->getUninstantiatedDefaultArg();
        clang::QualType otype = pd->getOriginalType();
        qDebug()<<otype.getAsString().data();
        udae->dumpColor();
        llvm::APSInt ival;
        bool bret;
        int nulltype = 0;
        clang::Expr::EvalResult eres, eres2;
        if (dae->isIntegerConstantExpr(ival, this->mtrunit->getASTContext())) {
            dps << QVariant((qlonglong)ival.getZExtValue());
        } else if (clang::isa<clang::CXXConstructExpr>(dae)) {
            // dps << eval_ctor(clang::cast<clang::CXXConstructExpr>(dae), this);
            dps << QVariant::fromValue(EvalType(dae, 0));
        } else if (dae->isCXX11ConstantExpr(mtrunit->getASTContext())) {
            bret = dae->EvaluateAsRValue(eres, mtrunit->getASTContext());
            qDebug()<<bret;
            eres.Val.dump();
            bret = dae->EvaluateAsInt(ival, mtrunit->getASTContext());
            qDebug()<<cnter<<bret<<ival.getLimitedValue() ;
            ival.dump();
            qDebug()<<"========";
            nulltype = dae->isNullPointerConstant(mtrunit->getASTContext(), 
                                                  clang::Expr::NullPointerConstantValueDependence::NPC_NeverValueDependent);
            if (nulltype == clang::Expr::NullPointerConstantKind::NPCK_ZeroLiteral) {
                dps << QVariant::fromValue((void*)0);
            } else {
                qDebug()<<"unknown eval result type:";
            }
        }
        else {
            qDebug()<<cnter<<dae->isCXX11ConstantExpr(mtrunit->getASTContext())
                    <<dae->isConstantInitializer(mtrunit->getASTContext(), true)
                    <<dae->isEvaluatable(mtrunit->getASTContext());
        }

    }

    qDebug()<<"dargs:"<<dps<<cnter<<dps.count();

    return true;
}

QVariant FrontEngine::get_method_return_type(clang::CXXMethodDecl *decl)
{
    QVariant vretype;
    QString retype;
    clang::QualType t = decl->getReturnType();
    retype = QString(t.getAsString().c_str());
    if (retype.startsWith("class ")) {
        retype = retype.right(retype.length() - 6);
    }
    vretype = retype;
    qDebug()<<"vretype:"<<vretype;
    // TODO maybe can convert to QMetaType

    return retype;
}

int FrontEngine::get_class_enum(clang::CXXRecordDecl *decl, QString enum_name)
{
    clang::DeclContext *dctx = decl->getDeclContext();
    // qDebug()<<"dctx:"<<dctx<<decl->hasDefinition();
    // CXXRecordDecl本身就是DeclContext

    clang::EnumConstantDecl *ecd = NULL;
    int cnter = 0;
    for (auto it = decl->decls_begin(); it != decl->decls_end(); it++, cnter++) {
        clang::Decl *d = *it;
        // qDebug()<<d->getDeclKindName();
        if (!llvm::isa<clang::EnumDecl>(d)) continue;

        bool found = false;
        auto ed = llvm::cast<clang::EnumDecl>(d);
        // qDebug()<<d->getDeclKindName()<<ed->getName().data();
        for (auto e: ed->enumerators()) {
            // qDebug()<<d->getDeclKindName()<<ed->getName().data()<<e->getName().data();
            if (enum_name == e->getName().data()) {
                qDebug()<<"found it:"<<enum_name;
                qDebug()<<d->getDeclKindName()<<ed->getName().data()<<e->getName().data();
                found = true;
                ecd = e;
                break;
            }
        }
        if (found) break;
    }
    // qDebug()<<"dctx:"<<dctx<<cnter;
    // qDebug()<<ecd;

    // 计算这个enum的值
    llvm::APSInt ev = ecd->getInitVal();
    // qDebug()<<ev.getLimitedValue();

    return ev.getLimitedValue();
    return -2;
}

// 查找Qt namespace的enum定义
int FrontEngine::get_qtns_enum(QString enum_name)
{
    // find Qt namespace
    clang::TranslationUnitDecl *udecl = this->mtrunit;
    Q_ASSERT(udecl != NULL);

    QVector<clang::NamespaceDecl*> nsdecls;
    int qtns_num = 0;
    for (auto it = udecl->decls_begin(); it != udecl->decls_end(); it++) {
        clang::Decl *decl = *it;
        clang::NamespaceDecl *nsdecl;

        QString declname;
        // qDebug()<<decl<<decl->getDeclKindName()<<decl->getKind();
        switch (decl->getKind()) {
        case clang::Decl::Namespace:
            nsdecl = llvm::cast<clang::NamespaceDecl>(decl);
            declname = nsdecl->getName().data();
            // qDebug()<<declname;
            if (declname == "Qt") {
                qDebug()<<"found it:"<<declname;
                qtns_num ++;
                nsdecls.append(nsdecl);
            }
            break;
        default: break;
        }
    }

    // find enum here

    bool found = false;
    clang::EnumConstantDecl* ecd = NULL;
    for (auto nsdecl: nsdecls) {
        for (auto d: nsdecl->decls()) {
            if (!llvm::isa<clang::EnumDecl>(d)) continue;

            auto ed = llvm::cast<clang::EnumDecl>(d);
            // qDebug()<<d->getDeclKindName()<<ed->getName().data();
            for (auto e: ed->enumerators()) {
                // qDebug()<<d->getDeclKindName()<<ed->getName().data()<<e->getName().data();
                if (enum_name == e->getName().data()) {
                    qDebug()<<"found it:"<<enum_name;
                    qDebug()<<d->getDeclKindName()<<ed->getName().data()<<e->getName().data();
                    found = true;
                    ecd = e;
                    break;
                }
            }
            if (found) break;
        }
        if (found) break;
    }

    if (!found) {
        qDebug()<<"enum not found:"<<enum_name;
        return -4;
    }

    // 计算这个enum的值
    llvm::APSInt ev = ecd->getInitVal();
    // qDebug()<<ev.getLimitedValue();

    return ev.getLimitedValue();
    return -3;
}

