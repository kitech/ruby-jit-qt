
#include <QtCore>

#include "llvm/IR/LLVMContext.h"
// #include "RemoteMemoryManager.h"
// #include "RemoteTarget.h"
// #include "RemoteTargetExternal.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/ExecutionEngine/JITMemoryManager.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/ObjectCache.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Object/Archive.h"
#include "llvm/Object/ObjectFile.h"
// #include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/Memory.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PluginLoader.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation.h"
#include <cerrno>

#include <llvm/Support/SourceMgr.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/PreprocessorOptions.h>
#include "llvm/Support/Host.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/ModuleLoader.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"

////////
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/LTO/LTOCodeGenerator.h"
#include "llvm/LTO/LTOModule.h"
#include "llvm/Target/TargetOptions.h"

#include "clvm.h"

// FIXME: MCTargetOptionsCommandFlags.h中几个函数的multiple definition问题
#include "clvm_letacy.cpp"


Clvm::Clvm() : QThread()
{
    static char *argv[] = {
        (char*)"prettystack"
    };
    static int argc = 1;

    sys::PrintStackTraceOnErrorSignal();
    llvm::PrettyStackTraceProgram X(argc, argv);

    init();
    initCompiler();
}

Clvm::~Clvm()
{
}

bool Clvm::init()
{
    // 是由于定义了链接参数：-Wl,--allow-multiple-definition引起的上面这个错误。
    // CommandLine Error: Option 'load' registered more than once!

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetDisassembler();

    return true;
}


bool Clvm::initCompiler()
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

bool Clvm::initExecutionEngine()
{
    return true;
}

llvm::GenericValue
Clvm::execute(QString &code, std::vector<llvm::GenericValue> &args, QString func_entry)
{
    llvm::GenericValue rgv;
    bool bret;

    const char *pcode = strdup(code.toLatin1().data());
    llvm::MemoryBuffer *mbuf = llvm::MemoryBuffer::getMemBuffer(pcode);
    clang::PreprocessorOptions &ppOpt = this->mcis->getPreprocessorOpts();
    ppOpt.addRemappedFile("flycode.cxx", mbuf);

    clang::EmitLLVMOnlyAction llvm_only_action;
    bret = mcis->ExecuteAction(llvm_only_action);
    llvm::Module *mod = llvm_only_action.takeModule();
    qDebug()<<"compile code done."<<mod;

    if (mod == NULL) {
    } else {
        rgv = run_module_func(mod, args, func_entry);
    }

    return rgv;
}

llvm::GenericValue 
Clvm::run_module_func(llvm::Module *mod, std::vector<llvm::GenericValue> &args, QString func_entry)
{
    // 只能放在这，run action之后，exeucte function之前
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    // llvm::InitializeNativeTargetDisassembler();

    // run module
    llvm::EngineBuilder eb(mod);
    eb.setUseMCJIT(true);

    llvm::ExecutionEngine *EE = eb.create();

    //*
    qDebug()<<"before load obj...";
    llvm::ErrorOr<llvm::object::ObjectFile *> obj = 
        llvm::object::ObjectFile::createObjectFile("/usr/lib/libQt5Sql.so");
    if (!obj) {
        qDebug()<<"create obj file error.";
    } else {
        // crash, 是不是因为当前已经是在.so中了呢？
        // EE->addObjectFile(std::unique_ptr<llvm::object::ObjectFile>(obj.get()));
    }
    // */


    EE->finalizeObject();
    EE->runStaticConstructorsDestructors(false);

    llvm::Function *etyfn = NULL;
    QString mangle_name;
    llvm::ValueSymbolTable &vst = mod->getValueSymbolTable();
    for (auto sit = vst.begin(); sit != vst.end(); sit++) {
        auto k = sit->first();
        auto v = sit->second;
        if (QString(v->getName().data()).startsWith("__PRETTY_FUNCTION__")) {
            continue;
        }
        // if (QString(v->getName().data()).indexOf("jit_main") >= 0) {
        if (QString(v->getName().data()).indexOf(func_entry) >= 0) {
            etyfn = mod->getFunction(v->getName());
            mangle_name = QString(v->getName().data());
            // break;
        }
        qDebug()<<""<<k.data()<<v->getName().data();
        //  v->dump();
    }
    qDebug()<<"our fun:"<<func_entry<<mangle_name<<etyfn;

    // std::vector<llvm::GenericValue> eeargs;
    llvm::GenericValue rgv = EE->runFunction(etyfn, args);

    EE->runStaticConstructorsDestructors(true);
    // cleanups

    qDebug()<<"run code done.";
    return rgv;
}

void Clvm::run()
{

}

void Clvm::deleteAsync(QString klass_name, void *obj)
{
    
}



