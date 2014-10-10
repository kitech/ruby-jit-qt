
/////////
#include <llvm/Support/Host.h>

#include <clang/AST/AST.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/APValue.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"


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
    clang::ASTContext &tctx = unit->getASTContext();
    clang::SourceManager &srcman = unit->getSourceManager();
    clang::FileManager &fman = unit->getFileManager();
    clang::TranslationUnitDecl *trud = tctx.getTranslationUnitDecl();

    if (trud == NULL) {
        qDebug()<<"load ast file error.";
        return false;
    }
    
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
    clang::CXXRecordDecl *recdecl = this->find_class_decl(yaklass);
    QVector<clang::CXXMethodDecl*> mthdecls = this->find_method_decls(recdecl, yaklass, method);

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
    clang::CXXRecordDecl *recdecl = this->find_class_decl(yaklass);
    QVector<clang::CXXMethodDecl*> mthdecls = this->find_method_decls(recdecl, yaklass, method);
    bool match = false;

    QVector<clang::CXXMethodDecl*> mats;
    for (clang::CXXMethodDecl *d: mthdecls) {
        match = this->method_match_by_uargs(d, yaklass, method, uargs);
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
    clang::CXXRecordDecl *recdecl = this->find_class_decl(yaklass);
    QVector<clang::CXXMethodDecl*> mthdecls = this->find_method_decls(recdecl, yaklass, method);

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

    QVariant tmp_retype = this->get_method_return_type(mthdecl);
    retype = tmp_retype;

    return retype.isValid();
    return false;
}


///// privates
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
    
    if (res_recdecl == NULL) {
        qDebug()<<"klass decl not found:";
        return NULL;
    }

    clang::CXXRecordDecl *recdecl = res_recdecl;
    return recdecl;
}

QVector<clang::CXXMethodDecl*> FrontEngine::find_method_decls(clang::CXXRecordDecl *decl,
                                                 QString klass, QString method)
{
    QVector<clang::CXXMethodDecl*> mdecls;

    clang::CXXRecordDecl *recdecl = decl;

    for (auto ait = recdecl->method_begin(); ait != recdecl->method_end(); ait++) {
        clang::CXXMethodDecl *mthdecl = *ait;
        // qDebug()<<"name .."<<mthdecl->getName().data();
        if (QString(mthdecl->getName().data()) == method) {
            qDebug()<<"found it, maybe rc:";
            // mthdecl->dump();
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
    
    int idx = 0;
    int pless = 0; //
    QVector<int> lessed;
    for (auto it = decl->param_begin(); it != decl->param_end(); it++, idx++) {
        clang::ParmVarDecl *pd = *it;
        clang::QualType ptype = pd->getType();
        QString tstr = ptype.getAsString().c_str(); // type str 

        if (uargs.count() > idx) {
            bool ok = false;
            switch ((int)uargs.at(idx).type()) {
            case QMetaType::Int: case QMetaType::UInt:
            case QMetaType::Long: case QMetaType::ULong:
            case QMetaType::LongLong: case QMetaType::ULongLong:
            case QMetaType::Short: case QMetaType::UShort:
                if (ptype->isIntegralOrEnumerationType()) ok = true;
                break;
            case QMetaType::Bool:
                if (ptype->isBooleanType()) ok = true;
                break;
            case QMetaType::QString:
                if (tstr.indexOf("QString") != -1 /* && tstr.indexOf('*') == -1*/) ok = true;
                break;
            case QMetaType::QChar:
                if (ptype->isCharType()) ok = true;
                break;

            default: qDebug()<<"unknown type:"<<uargs.at(idx).type()<<uargs.at(idx);
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

    double match_degree = idx == 0 ? 1.0 : (1.0*pless/idx);
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

bool FrontEngine::get_method_default_params(clang::CXXMethodDecl *decl, QVector<QVariant> &dparams)
{
    QVector<QVariant> &dps = dparams;

    auto eval_ctor = [](clang::CXXConstructExpr* expr) -> QVariant {
        clang::CXXConstructorDecl *decl = expr->getConstructor();
        QString klass_name = (decl->getName().data());
        if (klass_name == "QChar") {
            return QVariant(QChar(' '));
        }
        return QVariant(99813721);
    };

    for (auto it = decl->param_begin(); it != decl->param_end(); it++) {
        clang::ParmVarDecl *pd = *it;
        if (!pd->hasDefaultArg()) {          
            dps << QVariant();
            continue;
        }
        clang::Expr *dae = pd->getDefaultArg();
        llvm::APSInt ival;
        if (dae->isIntegerConstantExpr(ival, this->mtrunit->getASTContext())) {
            dps << QVariant((qlonglong)ival.getZExtValue());
        } else if (clang::isa<clang::CXXConstructExpr>(dae)) {
            dps << eval_ctor(clang::cast<clang::CXXConstructExpr>(dae));
        }
        else {
            qDebug()<<dae->isCXX11ConstantExpr(mtrunit->getASTContext())
                    <<dae->isConstantInitializer(mtrunit->getASTContext(), true)
                    <<dae->isEvaluatable(mtrunit->getASTContext());
        }

    }

    qDebug()<<"dargs:"<<dps;

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


