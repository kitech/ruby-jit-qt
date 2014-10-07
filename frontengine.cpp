
/////////
#include <llvm/Support/Host.h>

#include <clang/AST/AST.h>
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

bool FrontEngine::parseHeader(QString path)
{
    qDebug()<<path;

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
    units.insert(path, unit);

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
            qDebug()<<"name.."<<recdecl->getName().data();
            declname = QString(recdecl->getName().data());
            if (declname == "QString") {
                decl->dump();
            }
            break;
        default:
            break;
        }
    }


    
    return true;
}

