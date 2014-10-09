
/////////
#include <llvm/Support/Host.h>

#include <clang/AST/AST.h>
#include <clang/AST/Mangle.h>
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










