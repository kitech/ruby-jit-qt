
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
#include "llvm/ExecutionEngine/JITMemoryManager.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/ObjectCache.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
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

#include "frontengine.h"
#include "clvm_operator.h"
#include "clvm.h"

// FIXME: MCTargetOptionsCommandFlags.h中几个函数的multiple definition问题
#include "clvm_letacy.cpp"



llvm::GenericValue jit_execute_func(llvm::Module *mod, llvm::Function *func)
{
    llvm::GenericValue gv;

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    llvm::ExecutionEngine *EE = NULL;
    qDebug()<<"222222222";
    std::string Error;
    llvm::EngineBuilder builder(mod);
    qDebug()<<EE<<"aaa is sym search disabled:";
    builder.setUseMCJIT(true);
    // builder.setMArch("core2"); // 这种强制设置可能引起问题，使用三个InitailzeNativexxx函数初始化。

    EE = builder.create();
    qDebug()<<EE<<"is sym search disabled:"<<EE->isSymbolSearchingDisabled();

    //* 这些行还是有用的，否则在用Qt相关类时崩溃（在普通的print无问题）
    // 不过这些调用应用是每个函数都调用还是初始化调用一次呢。
    EE->finalizeObject();
    EE->runStaticConstructorsDestructors(false);

    int miret = -1;
    llvm::GenericValue rgv;
    std::vector<std::string> argv;
    std::vector<llvm::GenericValue> gargv;

    gv = EE->runFunction(func, gargv);

    // Run static destructors.
    EE->runStaticConstructorsDestructors(true);

    qDebug()<<"run done.";   

    return gv;
}

llvm::GenericValue jit_vm_execute(QString code, QVector<llvm::GenericValue> &envp)
{
    llvm::GenericValue gv;
    
    llvm::LLVMContext &ctx = llvm::getGlobalContext();
    llvm::Module *module = new llvm::Module("top", ctx);
    llvm::IRBuilder<> builder(ctx);

    // load module
    QFile fp("./jit_types.ll");
    fp.open(QIODevice::ReadOnly);
    QByteArray asm_data = fp.readAll();
    fp.close();

    const char *asm_str = strdup(asm_data.data());
    qDebug()<<"llstrlen:"<<strlen(asm_str);

    llvm::SMDiagnostic smdiag;
    llvm::Module *rmod = NULL;

    rmod = llvm::ParseAssemblyString(asm_str, NULL, smdiag, ctx);
    qDebug()<<"rmod="<<rmod;
    
    // test new operator
    llvm::Type *TQString = rmod->getTypeByName("class.YaQString");
    qDebug()<<"type:"<<TQString;
    TQString->dump();

    // entry func
    llvm::Function *entry_func = 
        llvm::Function::Create(llvm::FunctionType::get(builder.getVoidTy()->getPointerTo(), false),
                               llvm::Function::ExternalLinkage,
                               "yamain", module);
    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "eee", entry_func));

    /*
    // new op
    std::vector<llvm::Type*> fargs = {TQString->getPointerTo()};
    llvm::ArrayRef<llvm::Type*> rfargs(fargs);
    llvm::FunctionType *funt = llvm::FunctionType::get(builder.getVoidTy(), rfargs, false);
    llvm::Constant *func = module->getOrInsertFunction("_ZN9YaQStringC1Ev", funt);

    // new func
    std::vector<llvm::Type*> new_fargs = {builder.getInt32Ty()};
    llvm::ArrayRef<llvm::Type*> new_rfargs(new_fargs);
    llvm::FunctionType *new_funt = llvm::FunctionType::get(builder.getInt8Ty()->getPointerTo(), new_rfargs, false);
    llvm::Constant *new_func = module->getOrInsertFunction("_Znwj",new_funt);

    llvm::Value *val = builder.CreateAlloca(TQString->getPointerTo());
    llvm::Value *mval = builder.CreateCall(new_func, builder.getInt32(1));
    llvm::Value *cval = builder.CreateBitCast(mval, TQString->getPointerTo());
    llvm::Value *val2 = builder.CreateStore(cval, val);

    LoadInst *rval = builder.CreateLoad(val);
    builder.CreateCall(func, rval);
    // builder.CreateRet(builder.getInt32(123));
    builder.CreateRet(rval);
    */


    irop_new(ctx, builder, module, "aaa");

    irop_call(ctx, builder, module, 0, "a", "b");

    qDebug()<<"dumppp begin.";
    module->dump();
    qDebug()<<"dumppp end.";

    // run it yamain
    gv = jit_execute_func(module, entry_func);

    return gv;
}


static IROperator *irop = NULL;
static IROperator *getIROp()
{
    if (::irop == NULL) {
        ::irop = new IROperator();
    }
    return ::irop;
}

/*
  从llvm::GenericValue转换成这个方法的实际类型。
  还需要有很多特定的转换要处理。
 */
QVariant mapGV2Variant(QString klass, QString method, QString symbol_name, llvm::GenericValue gv)
{
    IROperator *irop = getIROp();
    QVector<QVariant> args;
    // QString retype = irop->resolve_return_type(klass, method, args, symbol_name);
    QVariant vretype;
    bool bret = irop->mfe->get_method_return_type(klass, method, args, symbol_name, vretype);
    QString retype = vretype.toString();
    // if retype is empty, maybe a ctor
    if (retype.isEmpty()) return QVariant();

    if (retype == "int" || retype == "uint" || retype == "long" || retype == "ulong"
        || retype == "qlonglong" || retype == "qulonglong"
        || retype == "short" || retype == "ushort" ) {
        return QVariant((qlonglong)llvm::GVTOP(gv));
    } else if (retype == "bool") {
        return QVariant((bool)llvm::GVTOP(gv));
    } else if (retype == "double" || retype == "float") {
        return QVariant((double)(long)llvm::GVTOP(gv));
    } else if (retype == "void") {
        return QVariant();
    }

    if (retype == "QString") {
        return QVariant(QString(*(QString*)llvm::GVTOP(gv)));
    }

    if (retype == "QString &") {
        return QVariant(QString(*(QString*)llvm::GVTOP(gv)));
    }

    return QVariant();
}

void *jit_vm_new(QString klass, QVector<QVariant> args)
{
    void *jo = NULL;
    IROperator *irop = getIROp();
    llvm::Module *module = irop->dmod;
    llvm::IRBuilder<> &builder = irop->builder;
    llvm::LLVMContext &ctx = irop->ctx;

    // entry func
    llvm::Function *entry_func = 
        llvm::Function::Create(llvm::FunctionType::get(builder.getVoidTy()->getPointerTo(), false),
                               llvm::Function::ExternalLinkage,
                               "yamain", module);
    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "eee", entry_func));

    irop->knew(klass);

    module->dump();

    llvm::GenericValue gv = jit_execute_func(module, entry_func);
    jo = llvm::GVTOP(gv);
    return jo;
}

QVariant jit_vm_call(void *kthis, QString klass, QString method, QVector<QVariant> args)
{

    IROperator *irop = getIROp();
    llvm::Module *module = irop->dmod;
    llvm::IRBuilder<> &builder = irop->builder;
    llvm::LLVMContext &ctx = irop->ctx;

    QString symbol_name;

    // entry func
    std::vector<llvm::Type *> entry_func_type = {builder.getVoidTy()->getPointerTo()};
    llvm::ArrayRef<llvm::Type*> ref_entry_func_type(entry_func_type);
    llvm::Function *entry_func = 
        llvm::Function::Create(llvm::FunctionType::get(builder.getVoidTy()->getPointerTo(), 
                                                       ref_entry_func_type, false),
                               llvm::Function::ExternalLinkage,
                               "yamain2", module);
    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "eee", entry_func));
    int i = 0;
    for (auto it = entry_func->arg_begin(); it != entry_func->arg_end(); it ++, i++) {
        llvm::Argument &a = *it;
        if (i == 0) a.setName("kthis");
        else a.setName(QString("fa%1").arg(i).toStdString());
    }

    irop->call(kthis, klass, method, args, symbol_name);

    module->dump();
    qDebug()<<"============module dump end.";

    llvm::GenericValue gv = jit_execute_func(module, entry_func);
    int iret = gv.IntVal.getZExtValue();
    qDebug()<<"raw ret:"<<iret<<llvm::GVTOP(gv);

    // how to known return type
    // 需要把gv解释并转换为为什么类型呢。
    // 怎么转呢？
    // GenericValue要求首先要知道返回值的类型才能解释
    int tyno = QMetaType::Int;
    switch (tyno) {
    case QMetaType::Int: 
        iret = (long)(llvm::GVTOP(gv));
        // return QVariant(iret);
    default:
        break;
    }

    qDebug()<<"symbol name:"<<symbol_name;
    QVariant vret =  mapGV2Variant(klass, method, symbol_name, gv);
    entry_func->deleteBody();  // 删除函数体
    entry_func->removeFromParent(); // 删除整个函数
    // entry_func->eraseFromParent(); //  清理干净, 再执行这个就crash，为什么提供了这几个不同的方法呢？
    // entry_func->replaceAllUsesWith(llvm::UndefValue::get(entry_func->getType()));
    // entry_func->dropAllReferences();
    delete entry_func;
    // qDebug()<<"===============";
    // module->dump();
    // qDebug()<<"===============";

    return vret;
    
    return QVariant();
}

//////////////////////////////////
/////
//////////////////////////////////
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
Clvm::execute2(llvm::Module *mod, QString func_entry)
{
    // 加载额外模块
    static llvm::Module *jit_types_mod = NULL;
    auto load_jit_types_module = []() -> llvm::Module* {
        llvm::LLVMContext *ctx = new llvm::LLVMContext();
        llvm::Module *module = new llvm::Module("jit_types_in_vme", *ctx);

        // load module
        QFile fp("./metalize/jit_types.ll");
        fp.open(QIODevice::ReadOnly);
        QByteArray asm_data = fp.readAll();
        fp.close();

        char *asm_str = strdup(asm_data.data());
        qDebug()<<"llstrlen:"<<strlen(asm_str);

        llvm::SMDiagnostic smdiag;
        llvm::Module *rmod = NULL;

        rmod = llvm::ParseAssemblyString(asm_str, NULL, smdiag, *ctx);
        qDebug()<<"rmod="<<rmod;
        free(asm_str);

        return rmod;
    };

    if (jit_types_mod == NULL) {
        jit_types_mod = load_jit_types_module();
    }

    // 只能放在这，run action之后，exeucte function之前
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    // llvm::InitializeNativeTargetDisassembler();

    // run module
    llvm::EngineBuilder eb(mod);
    eb.setUseMCJIT(true);

    llvm::ExecutionEngine *EE = eb.create();
    EE->addModule(jit_types_mod);

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

    std::vector<llvm::GenericValue> args;
    llvm::GenericValue rgv = EE->runFunction(etyfn, args);

    EE->runStaticConstructorsDestructors(true);
    // cleanups

    qDebug()<<"run code done.";
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


