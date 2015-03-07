
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

#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/LTO/LTOCodeGenerator.h"
#include "llvm/LTO/LTOModule.h"
#include "llvm/Target/TargetOptions.h"

#include "clvm_letacy.h"


// 参考：lli.cpp的实现
static void run_module(llvm::Module *mod, llvm::GenericValue &gv, QVector<llvm::GenericValue> &args)
{
    // debug_module(mod);
    qDebug()<<"1111111";
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();


    llvm::ExecutionEngine *EE = NULL;
    qDebug()<<"222222222";
    std::string Error;
    std::unique_ptr<llvm::Module> tmod(mod);
    llvm::EngineBuilder builder(std::move(tmod));
    qDebug()<<EE<<"aaa is sym search disabled:";
    // builder.setUseMCJIT(true);
    // builder.setMArch("core2"); // 这种强制设置可能引起问题，使用三个InitailzeNativexxx函数初始化。

    EE = builder.create();
    qDebug()<<EE<<"is sym search disabled:"<<EE->isSymbolSearchingDisabled();

    //* 这些行还是有用的，否则在用Qt相关类时崩溃（在普通的print无问题）
    // 不过这些调用应用是每个函数都调用还是初始化调用一次呢。
    EE->finalizeObject();
    EE->runStaticConstructorsDestructors(false);
    /*
    for (llvm::Module::iterator I = mod->begin(), E = mod->end(); I != E; ++I) {
        llvm::Function *Fn = &*I;
        if (!Fn->isDeclaration())
            EE->getPointerToFunction(Fn);
    }
    */
    // */
    int miret = -1;
    llvm::GenericValue rgv;
    std::vector<std::string> argv;
    std::vector<llvm::GenericValue> gargv;

    foreach (auto a, args) {
        gargv.push_back(a);
    }
    if (gargv.size() > 0) {
        qDebug()<<"view before funcall:"<<GVTOP(gargv.at(0))<<GVTOP(args.at(0));
    }

    // llvm::Function *vfun = mod->getFunction("_Z5amainv");
    // EE->runFunctionAsMain(vfun, argv, 0);

    // llvm::Function *vfun_main = mod->getFunction("main");
    // if (vfun_main) miret = EE->runFunctionAsMain(vfun_main, argv, 0);
    // qDebug()<<"miret:"<<miret;

    /*
    llvm::Function *vfun_yamain = mod->getFunction("_Z6yamainv");
    qDebug()<<"yamain:"<<vfun_yamain;
    EE->runFunctionAsMain(vfun_yamain, argv, 0);
    */

    llvm::Function *vfun_jit_main = mod->getFunction("_Z8jit_mainv");
    qDebug()<<"jit_main:"<<vfun_jit_main;
    if (vfun_jit_main) rgv = EE->runFunction(vfun_jit_main, gargv);
    qDebug()<<"miret:"<<miret<<llvm::GVTOP(rgv);
    gv = rgv;

    if (vfun_jit_main == NULL) {

        if (gargv.size() > 0) {
            qDebug()<<"view before funcall222:"<<GVTOP(gargv.at(0))<<GVTOP(args.at(0));
        }

        vfun_jit_main = mod->getFunction("_Z8jit_mainiPv");
        qDebug()<<"jit_main:"<<vfun_jit_main;
        if (vfun_jit_main) rgv = EE->runFunction(vfun_jit_main, gargv);
        qDebug()<<"miret:"<<miret<<llvm::GVTOP(rgv);
        gv = rgv;
    }

    //*
    // Run static destructors.
    EE->runStaticConstructorsDestructors(true);
    // */

    qDebug()<<"run done.";
}


// // 参考：lli.cpp的实现
// static void run_module(llvm::Module *mod, llvm::GenericValue &gv,
//                        QVector<llvm::GenericValue> &args)
// {
//     // debug_module(mod);
//     qDebug()<<"1111111";
//     llvm::InitializeNativeTarget();
//     llvm::InitializeNativeTargetAsmPrinter();
//     llvm::InitializeNativeTargetAsmParser();


//     llvm::ExecutionEngine *EE = NULL;
//     qDebug()<<"222222222";
//     std::string Error;
//     llvm::EngineBuilder builder(mod);
//     qDebug()<<EE<<"aaa is sym search disabled:";
//     builder.setUseMCJIT(true);
//     // builder.setMArch("core2"); // 这种强制设置可能引起问题，使用三个InitailzeNativexxx函数初始化。

//     EE = builder.create();
//     qDebug()<<EE<<"is sym search disabled:"<<EE->isSymbolSearchingDisabled();

//     //* 这些行还是有用的，否则在用Qt相关类时崩溃（在普通的print无问题）
//     // 不过这些调用应用是每个函数都调用还是初始化调用一次呢。
//     EE->finalizeObject();
//     EE->runStaticConstructorsDestructors(false);
//     /*
//     for (llvm::Module::iterator I = mod->begin(), E = mod->end(); I != E; ++I) {
//         llvm::Function *Fn = &*I;
//         if (!Fn->isDeclaration())
//             EE->getPointerToFunction(Fn);
//     }
//     */
//     // */
//     int miret = -1;
//     llvm::GenericValue rgv;
//     std::vector<std::string> argv;
//     std::vector<llvm::GenericValue> gargv;

//     foreach (auto a, args) {
//         gargv.push_back(a);
//     }
//     if (gargv.size() > 0) {
//         qDebug()<<"view before funcall:"<<GVTOP(gargv.at(0))<<GVTOP(args.at(0));
//     }

//     // llvm::Function *vfun = mod->getFunction("_Z5amainv");
//     // EE->runFunctionAsMain(vfun, argv, 0);

//     // llvm::Function *vfun_main = mod->getFunction("main");
//     // if (vfun_main) miret = EE->runFunctionAsMain(vfun_main, argv, 0);
//     // qDebug()<<"miret:"<<miret;

//     /*
//     llvm::Function *vfun_yamain = mod->getFunction("_Z6yamainv");
//     qDebug()<<"yamain:"<<vfun_yamain;
//     EE->runFunctionAsMain(vfun_yamain, argv, 0);
//     */

//     QString func_entry = "jit_main";
//     llvm::Function *etyfn = NULL;
//     QString mangle_name;
//     llvm::ValueSymbolTable &vst = mod->getValueSymbolTable();
//     for (auto sit = vst.begin(); sit != vst.end(); sit++) {
//         auto k = sit->first();
//         auto v = sit->second;
//         if (QString(v->getName().data()).startsWith("__PRETTY_FUNCTION__")) {
//             continue;
//         }
//         // if (QString(v->getName().data()).indexOf("jit_main") >= 0) {
//         if (QString(v->getName().data()).indexOf(func_entry) >= 0) {
//             etyfn = mod->getFunction(v->getName());
//             mangle_name = QString(v->getName().data());
//             break;
//         }
//         // qDebug()<<""<<k.data()<<v->getName().data();
//         //  v->dump();
//     }
//     qDebug()<<"our fun:"<<func_entry<<mangle_name<<etyfn;

//     // std::vector<llvm::GenericValue> eeargs;
//     rgv = EE->runFunction(etyfn, gargv);
//     gv = rgv;

//     if (0) {
//         llvm::Function *vfun_jit_main = mod->getFunction("_Z8jit_mainv");
//         qDebug()<<"jit_main:"<<vfun_jit_main;
//         if (vfun_jit_main) rgv = EE->runFunction(vfun_jit_main, gargv);
//         qDebug()<<"miret:"<<miret<<llvm::GVTOP(rgv);
//         gv = rgv;

//         if (vfun_jit_main == NULL) {

//             if (gargv.size() > 0) {
//                 qDebug()<<"view before funcall222:"<<GVTOP(gargv.at(0))<<GVTOP(args.at(0));
//             }

//             vfun_jit_main = mod->getFunction("_Z8jit_mainiPv");
//             qDebug()<<"jit_main:"<<vfun_jit_main;
//             if (vfun_jit_main) rgv = EE->runFunction(vfun_jit_main, gargv);
//             qDebug()<<"miret:"<<miret<<llvm::GVTOP(rgv);
//             gv = rgv;
//         }
//     }
//     //*
//     // Run static destructors.
//     EE->runStaticConstructorsDestructors(true);
//     // */

//     qDebug()<<"run done.";
// }


static void debug_module(llvm::Module *mod)
{
    qDebug()<<"identifier:"<<mod->getModuleIdentifier().c_str();
    qDebug()<<"target triple:"<<mod->getTargetTriple().c_str();
    qDebug()<<"inline asm:" << mod->getModuleInlineAsm().c_str();

    llvm::GlobalValue * mgv = mod->getNamedValue("_ZL1s");
    qDebug()<<"mgv:s:"<<mgv;
    // mod->dump();
    qDebug()<<"data layout str:"<<mod->getDataLayoutStr().c_str();

    llvm::Function *vfun = mod->getFunction("main");
    qDebug()<<"main func:"<<vfun;
}


std::string GetExecutablePath(const char *Argv0) {
  // This just needs to be some symbol in the binary; C++ doesn't
  // allow taking the address of ::main however.
  void *MainAddr = (void*) (intptr_t) GetExecutablePath;
  return llvm::sys::fs::getMainExecutable(Argv0, MainAddr);
}

static llvm::Module *compile_string(int argc, char **argv, const char *code,
                                    llvm::GenericValue &gv, 
                                    QVector<llvm::GenericValue> &args)
{

    llvm::Module *ccm = NULL;
    clang::CompilerInvocation *civ;
    clang::CompilerInstance cis;
    bool bret;

    std::vector<const char *> xargv;
    // xargv.push_back("myjitclangpp");
    // xargv.push_back("-x");
    // xargv.push_back("c++");
    // xargv.push_back("-S");
    // xargv.push_back("-emit-llvm");
    // xargv.push_back("-arch");
    // xargv.push_back("native");
    // xargv.push_back("-t123");
    // xargv.push_back("-t456");
    xargv.push_back("forlli.cpp");
    // xargv.push_back("-t789");
    // xargv.push_back("-t012");

    char file_entry[100] = {0};
    const char *targv[] = {
        "myjitclangpp", file_entry,
        "--",
        "-x", "c++",
        "-s", "-emit-llvm",
    };
    int targc = 7;
    strcpy((char *)file_entry, "forlli.cpp");

    cis.createDiagnostics();
    cis.createFileManager();
    cis.createSourceManager(cis.getFileManager());
    // cis.createPreprocessor(clang::TranslationUnitKind TUKind);
    // cis.createModuleManager();
    cis.createFrontendTimer();
    // cis.createASTContext();

    void *MainAddr = (void*) (intptr_t) GetExecutablePath;
    std::string Path = GetExecutablePath(argv[0]);

    clang::driver::Driver theDriver(Path, llvm::sys::getProcessTriple(), cis.getDiagnostics());
    theDriver.setTitle("myjitclangpp");
    theDriver.setCheckInputsExist(false);

    llvm::SmallVector<const char*, 16> Args(argv, argv+argc);
    Args.push_back("-S");
    clang::driver::Compilation *C = theDriver.BuildCompilation(Args);

    const clang::driver::JobList &Jobs = C->getJobs();
    const clang::driver::Command *Cmd = llvm::cast<clang::driver::Command>(&(*Jobs.begin()));
    const clang::driver::ArgStringList &CCArgs = Cmd->getArguments();

    civ = new clang::CompilerInvocation();
    bret = clang::CompilerInvocation::CreateFromArgs(*civ, const_cast<const char**>(CCArgs.data()),
                                                     const_cast<const char**>(CCArgs.data()) + CCArgs.size(),
                                                     cis.getDiagnostics());
    Jobs.Print(llvm::errs(), "\n", true);

    // qDebug()<<bret<<civ->getModuleHash().c_str();
    cis.setInvocation(civ);

    clang::CodeGenOptions &cgopt = cis.getCodeGenOpts();
    clang::FrontendOptions &ftopt = cis.getFrontendOpts();
    ftopt.ProgramAction = clang::frontend::EmitLLVMOnly;
    clang::TargetOptions &tgopt = cis.getTargetOpts();
    // tgopt.Triple = "i386-pc-linux-gnu";
    qDebug()<<"triple:"<<tgopt.Triple.c_str()
            <<"cpu:"<<tgopt.CPU.c_str()
            <<"abi:"<<tgopt.ABI.c_str();

    std::unique_ptr<llvm::MemoryBuffer> mbuf = llvm::MemoryBuffer::getMemBuffer(code);
    clang::PreprocessorOptions &ppopt = cis.getPreprocessorOpts();
    //    ppopt.addRemappedFile("flycode.cxx", code);
    ppopt.addRemappedFile("flycode.cxx", mbuf.release());

    

    clang::EmitLLVMOnlyAction llvm_only_action;
    bret = cis.ExecuteAction(llvm_only_action);
    qDebug()<<bret;
    std::unique_ptr<llvm::Module> mod = llvm_only_action.takeModule();
    qDebug()<<"comipiled mod:"<<mod.get();
    // mod->dump();
    
    run_module(mod.get(), gv, args);
    qDebug()<<"compile code done";
    return ccm;
}

const char *dcode = NULL;
/*
  目标，即时编译一段文本代码并执行，结果要能传递到当前进程中。
  执行：./jitrun flycode.cxx  -I/usr/lib/clang/3.5.0/include
 */
int intern_main(int argc, char **argv, llvm::GenericValue &gv, QVector<llvm::GenericValue> &args)
{
    const char *code = "#include <stdio.h>\n"
        "\nint main() { printf (\"hello IR JIT.\"); return 56; }"
        "\nint yamain() { return main(); }";
    code = dcode;

    QFile fp("./demos/forlli.ll");
    fp.open(QIODevice::ReadOnly);
    QByteArray asm_data = fp.readAll();
    fp.close();

    const char *asm_str = strdup(asm_data.data());
    qDebug()<<strlen(asm_str);

    llvm::LLVMContext &gctx = llvm::getGlobalContext();
    llvm::SMDiagnostic smdiag;
    llvm::Module *pmod = NULL;
    std::unique_ptr<llvm::Module> rmod;

    rmod = llvm::parseAssemblyString(asm_str, /*pmod,*/ smdiag, gctx);
    qDebug()<<rmod.get()<<pmod;
    debug_module(rmod.get());

    // run_module(rmod);

    llvm::Module *ccm = compile_string(argc, argv, code, gv, args);

    return 0;
}

llvm::GenericValue vm_execute(QString code, QVector<llvm::GenericValue> &envp)
{
    char *argv[] = {
        (char*)"myjitqtrunner", (char*)"flycode.cxx",
        (char*)"-fPIC", (char*)"-x", (char*)"c++", 
        (char*)"-I/usr/include/qt", (char*)"-I/usr/include/qt/QtCore",
    };
    int argc = 7;

    dcode = strdup(code.toStdString().c_str());

    llvm::GenericValue ergv;
    intern_main(argc, argv, ergv, envp);

    return ergv;
}

