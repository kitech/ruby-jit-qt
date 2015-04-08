#include "fix_clang_undef_eai.h"

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
// #include "llvm/ExecutionEngine/JIT.h" // depcreated 
#include "llvm/ExecutionEngine/JITEventListener.h"
// #include "llvm/ExecutionEngine/JITMemoryManager.h"
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

// 为什么CommandFlags.h在两个不同cpp文件中包含会引起ABIname重定义问题呢？
// #include "llvm/CodeGen/CommandFlags.h"
#include "llvm/LTO/LTOCodeGenerator.h"
#include "llvm/LTO/LTOModule.h"
#include "llvm/Target/TargetOptions.h"

#include "ExecutionEngine/MCJIT/MCJIT.h"

////////////////
#include "frontengine.h"
// #include "clvm_operator.h"
// #include "clvm.h"
#include "ivm/dbghelper.h"
#include "ivm/clvmjitlistener.h"


// FIXME: MCTargetOptionsCommandFlags.h中几个函数的multiple definition问题
// #include "clvm_letacy.cpp"

#include "ivm/modulemanager.h"
#include "ivm/codeunit.h"

#include "clvmengine.h"

//////////////////////////////////
/////
//////////////////////////////////
/*
 * TODO 这个类需要做成单实例模式的。
 */
ClvmEngine::ClvmEngine() : QThread()
{
    static char *argv[] = {
        (char*)"prettystack"
    };
    static int argc = 1;

    llvm::sys::PrintStackTraceOnErrorSignal();
    llvm::PrettyStackTraceProgram X(argc, argv);

    init();
    initCompiler();

    initExecutionEngine();
}

ClvmEngine::~ClvmEngine()
{
}

bool ClvmEngine::init()
{
    // 是由于定义了链接参数：-Wl,--allow-multiple-definition引起的上面这个错误。
    // CommandLine Error: Option 'load' registered more than once!

    static bool llvm_init_native_done = false;

    // TODO 是否需要使用线程锁呢？
    if (!llvm_init_native_done) {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();
        llvm::InitializeNativeTargetDisassembler();
        
        llvm_init_native_done = true;
    }
    
    return true;
}


bool ClvmEngine::initCompiler()
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
    clang::driver::Driver *drv =
        new clang::driver::Driver(path, llvm::sys::getProcessTriple(), cis->getDiagnostics());
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
    const clang::driver::Command *Cmd = llvm::cast<clang::driver::Command>(&(*Jobs.begin()));
    const clang::driver::ArgStringList &CCArgs = Cmd->getArguments();

    // clang::CompilerInvocation *civ = new clang::CompilerInvocation();
    bool bret = clang::CompilerInvocation::CreateFromArgs(*civ,
                                                          const_cast<const char**>(CCArgs.data()),
                                                          const_cast<const char**>(CCArgs.data()) + CCArgs.size(),
                                                          cis->getDiagnostics());

    cis->setInvocation(civ);

    return true;
}

bool ClvmEngine::initExecutionEngine()
{
    // run module
    std::string dmname = "dummyee";
    llvm::StringRef rdmname(dmname);
    llvm::LLVMContext *ctx = new llvm::LLVMContext();
    llvm::LLVMContext &rctx = *ctx;
    
    llvm::Module *mod = new llvm::Module(rdmname, rctx);
    std::unique_ptr<llvm::Module> tmod(mod);
    llvm::EngineBuilder eb(std::move(tmod));
    
    std::string errstr;    
    eb.setErrorStr(&errstr);
    // eb.setUseMCJIT(true);

    llvm::ExecutionEngine *EE = eb.create();

    // ClvmJitListener *lsner = new ClvmJitListener();
    if (!mlsner) mlsner = ClvmJitListener::inst();
    EE->RegisterJITEventListener(mlsner);

    //*
    qDebug()<<"before load obj...";
    llvm::ErrorOr<llvm::object::OwningBinary<llvm::object::ObjectFile> > obj = 
        llvm::object::ObjectFile::createObjectFile("/usr/lib/libQt5Sql.so");
    if (!obj) {
        qDebug()<<"create obj file error.";
    } else {
        // crash, 是不是因为当前已经是在.so中了呢？
        // EE->addObjectFile(std::unique_ptr<llvm::object::ObjectFile>(obj.get()));
    }
    // */

    mee = EE;
    /*
    if (llvm::isa<llvm::MCJIT>(EE)) {
        llvm::MCJIT *jee = llvm::cast<llvm::MCJIT>(EE);
    }
    */
    llvm::MCJIT *jee = llvm::cast<llvm::MCJIT>(EE);
    // mman = new ModuleManager(EE); 
    mman = ModuleManager::inst();
    mman->mee = EE;
    
    return true;
}


llvm::ExecutionEngine *ClvmEngine::createEE(llvm::Module *mod)
{
    // run module
    std::unique_ptr<llvm::Module> tmod(mod);
    llvm::EngineBuilder eb(std::move(tmod));
    
    std::string errstr;    
    eb.setErrorStr(&errstr);
    // eb.setUseMCJIT(true);

    llvm::ExecutionEngine *EE = eb.create();

    // ClvmJitListener *lsner = new ClvmJitListener();
    if (!mlsner) mlsner = ClvmJitListener::inst();
    EE->RegisterJITEventListener(mlsner);

    return EE;
}

// 执行IR代码中的一个入口函数
llvm::GenericValue 
ClvmEngine::execute2(llvm::Module *mod, QString func_entry)
{
    // return execute3(mod, func_entry);
    
    // 只能放在这，run action之后，exeucte function之前
    // 这段代码现在不需要固定放在这个位置了，已经正常使用init()中的片段。
    // 也可能和当前类只使用了一个实例有关,这也就要求该必须是单实例的了。
    // llvm::InitializeNativeTarget();
    // llvm::InitializeNativeTargetAsmPrinter();
    // llvm::InitializeNativeTargetAsmParser();
    // llvm::InitializeNativeTargetDisassembler();

    // run module
    std::string errstr;
    std::unique_ptr<llvm::Module> tmod(mod);
    llvm::EngineBuilder eb(std::move(tmod));
    eb.setErrorStr(&errstr);
    // eb.setUseMCJIT(true);

    llvm::ExecutionEngine *EE = eb.create();

    // ClvmJitListener *lsner = new ClvmJitListener();
    if (!mlsner) mlsner = ClvmJitListener::inst();
    EE->RegisterJITEventListener(mlsner);

    //*
    qDebug()<<"before load obj...";
    llvm::ErrorOr<llvm::object::OwningBinary<llvm::object::ObjectFile> > obj = 
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
        if (QString(v->getName().data()).startsWith("_Z15__jit_main_tmplv")) {
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

    std::vector<llvm::GenericValue> args;
    llvm::GenericValue rgv = EE->runFunction(etyfn, args);
    qDebug()<<"err:"<<errstr.c_str();
    
    EE->runStaticConstructorsDestructors(true);
    // cleanups

    qDebug()<<"run code done."<<llvm::GVTOP(rgv)<<rgv.IntVal.getZExtValue();
    return rgv;
}

// 使用类变量mee多次执行IR代码。
llvm::GenericValue ClvmEngine::execute3(llvm::Module */*mod*/, QString func_entry, CodeUnit *cu)
{
    // llvm::ExecutionEngine *EE = mee;
    llvm::Module *qtmod = cu->qtmod;
    llvm::Module *remod = cu->remod;
    QString modname = QString::fromStdString(qtmod->getName().str());

    llvm::ExecutionEngine *EE = mman->hasEE(modname) ? mman->getEE(modname) : this->createEE(qtmod);
    
    mman->add(QString::fromStdString(qtmod->getName().str()), qtmod);
    mman->addEE(QString::fromStdString(qtmod->getName().str()), EE);
    // mman->add(QString::fromStdString(remod->getName().str()), remod);
    std::unique_ptr<llvm::Module> tremod(remod);
    EE->addModule(std::move(tremod));
    assert(tremod.get() == NULL);
    
    EE->finalizeObject();
    EE->runStaticConstructorsDestructors(false);

    llvm::Function *etyfn = NULL;
    QString mangle_name;
    llvm::ValueSymbolTable &vst = remod->getValueSymbolTable();
    for (auto sit = vst.begin(); sit != vst.end(); sit++) {
        auto k = sit->first();
        auto v = sit->second;
        if (QString(v->getName().data()).startsWith("__PRETTY_FUNCTION__")) {
            continue;
        }
        if (QString(v->getName().data()).startsWith("_Z15__jit_main_tmplv")) {
            continue;
        }
        // if (QString(v->getName().data()).indexOf("jit_main") >= 0) {
        if (QString(v->getName().data()).indexOf(func_entry) >= 0) {
            etyfn = remod->getFunction(v->getName());
            mangle_name = QString(v->getName().data());
            // break;
        }
        qDebug()<<""<<k.data()<<v->getName().data();
        //  v->dump();
    }
    qDebug()<<"our fun:"<<func_entry<<mangle_name<<etyfn;

    // 由于remod还没有完成，暂时忽略新的执行方式。
    QString mangle_name2 = QString::fromStdString(remod->getName().str());
    llvm::Function *etyfn2 = remod->getFunction(mangle_name2.toStdString());
    qDebug()<<"our fun:"<<func_entry<<mangle_name2<<etyfn2;
    
    std::vector<llvm::GenericValue> args;
    llvm::GenericValue rgv = EE->runFunction(etyfn2, args);
    // qDebug()<<"err:"<<errstr.c_str();
    
    EE->runStaticConstructorsDestructors(true);
    // cleanups
    bool bret = false;
    // bret |= mman->remove(QString::fromStdString(remod->getName().str()));
    // bret |= mman->remove(QString::fromStdString(mod->getName().str()));
    EE->removeModule(remod);
    qDebug()<<bret;

    qDebug()<<"run code done."<<llvm::GVTOP(rgv)<<rgv.IntVal.getZExtValue();
    return rgv;
}

// 执行一段C++ Qt源代码，类似于使用命令行编译这段代码为IR代码，再使用EE执行。
// 这个使用了高级函数，灵活性和功能特征比较弱
llvm::GenericValue
ClvmEngine::execute(QString &code, std::vector<llvm::GenericValue> &args, QString func_entry)
{
    llvm::GenericValue rgv;
    bool bret;

    const char *pcode = strdup(code.toLatin1().data());
    std::unique_ptr<llvm::MemoryBuffer> mbuf = llvm::MemoryBuffer::getMemBuffer(pcode);
    clang::PreprocessorOptions &ppOpt = this->mcis->getPreprocessorOpts();
    ppOpt.addRemappedFile("flycode.cxx", mbuf.release());

    clang::EmitLLVMOnlyAction llvm_only_action;
    bret = mcis->ExecuteAction(llvm_only_action);
    std::unique_ptr<llvm::Module> mod = llvm_only_action.takeModule();
    qDebug()<<"compile code done."<<mod.get();

    if (mod == NULL) {
    } else {
        rgv = run_module_func(mod.get(), args, func_entry);
    }

    return rgv;
}

llvm::GenericValue 
ClvmEngine::run_module_func(llvm::Module *mod, std::vector<llvm::GenericValue> &args, QString func_entry)
{
    // 只能放在这，run action之后，exeucte function之前
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    // llvm::InitializeNativeTargetDisassembler();

    // run module
    std::unique_ptr<llvm::Module> tmod(mod);
    llvm::EngineBuilder eb(std::move(tmod));
    // eb.setUseMCJIT(true);

    llvm::ExecutionEngine *EE = eb.create();

    //*
    qDebug()<<"before load obj...";
    llvm::ErrorOr<llvm::object::OwningBinary<llvm::object::ObjectFile> > obj = 
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

void ClvmEngine::run()
{

}

void ClvmEngine::deleteAsync(QString klass_name, void *obj)
{
    
}

