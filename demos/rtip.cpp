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

////////
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/LTO/LTOCodeGenerator.h"
#include "llvm/LTO/LTOModule.h"
#include "llvm/Target/TargetOptions.h"


#include <QtCore>
////////

int main(int argc, const char *argv[])
{
    
    clang::CompilerInstance ci;
    clang::CompilerInvocation *civ = new clang::CompilerInvocation();
    bool bret = false;
    llvm::Module *module = NULL;

    ci.createDiagnostics();
    ci.createFileManager();


    // Clang tooling mode, ClangTool has not FrontendAction::takeModule() feature.
    if (0) {
        static llvm::cl::OptionCategory MyToolCategory("my-tool options");
        clang::tooling::CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
        clang::tooling::CompilationDatabase &cdb = OptionsParser.getCompilations();

        clang::tooling::ClangTool Tool(cdb, OptionsParser.getSourcePathList());

        auto llvm_act = clang::tooling::newFrontendActionFactory<clang::EmitLLVMOnlyAction>();        

        // bret = Tool.run(clang::tooling::newFrontendActionFactory<clang::DumpTokensAction>().get());
        // qDebug()<<bret;
        // bret = Tool.run(clang::tooling::newFrontendActionFactory<clang::DumpModuleInfoAction>().get());
        // qDebug()<<bret;

        bret = Tool.run(llvm_act.get());
        module = ((clang::EmitLLVMOnlyAction*)llvm_act.get()->create())->takeModule();
        qDebug()<<bret<<module;

        module->dump();
    }

    // invocation mode, ok. but can not pass arguments like -I/usr/include
    if (1) {
        std::vector<const char *> args;
        args.push_back("./demos/rtc_code1.cpp");

        args = {argv[1]};

        args.clear();
        for (int i = 1; i < argc; i++) {
            args.push_back(argv[1]);
        }
        qDebug()<<"args: size:"<<args.size();

        // civ = clang::createInvocationFromCommandLine(args);
        bret = clang::CompilerInvocation::CreateFromArgs(*civ, &args[0], &args[argc-1], ci.getDiagnostics());
        qDebug()<<"ca inv:"<<bret;
        ci.setInvocation(civ);

        civ->getHeaderSearchOpts().AddSystemHeaderPrefix("/usr/include", true);
        qDebug()<<"hehe,"<<civ->getHeaderSearchOpts().ResourceDir.c_str()
                << civ->getHeaderSearchOpts().UseBuiltinIncludes
                << civ->getHeaderSearchOpts().UseStandardSystemIncludes
                << civ->getHeaderSearchOpts().UseStandardCXXIncludes
                << civ->getHeaderSearchOpts().UserEntries.size()
                << civ->getHeaderSearchOpts().SystemHeaderPrefixes.size();

        qDebug()<<"hehe2,"<<ci.getHeaderSearchOpts().ResourceDir.c_str()
                << ci.getHeaderSearchOpts().UseBuiltinIncludes
                << ci.getHeaderSearchOpts().UseStandardSystemIncludes
                << ci.getHeaderSearchOpts().UseStandardCXXIncludes
                << ci.getHeaderSearchOpts().UserEntries.size()
                << ci.getHeaderSearchOpts().SystemHeaderPrefixes.size();


        clang::EmitLLVMOnlyAction llvm_act;
        clang::DumpModuleInfoAction act;
        qDebug()<<"exec:...";
        bret = ci.ExecuteAction(act);
        qDebug()<<"exec:"<<bret;
        bret = ci.ExecuteAction(llvm_act);
        module = llvm_act.takeModule();
        qDebug()<<"exec:"<<bret<<module;

        for (llvm::Module::FunctionListType::iterator i = module->getFunctionList().begin();
             i != module->getFunctionList().end(); ++i)
            printf("%s\n", i->getName().str().c_str());

        module->dump();

        /*
        auto flist = module->getFunctionList();
        std::for_each(flist.begin(), flist.end(), [](auto item) {
                qDebug()<<item->getName().str().c_str();
            });
        */

    }

    qDebug()<<"done";
    return 0;

    // example args
    const char *targv[] = {
        "iprtc", "/home/gzleo/opensource/qtbindings/handby/demos/rtc_code1.cpp", 
        "--",
        "-x", "c++",
        "-fPIC", "-I/usr/include/linux" "-I/usr/include/qt", "-I/usr/include/qt/QtCore"
    };
    int targc = 9;

    // targv = argv;
    // targc = argc;

    {
        static llvm::cl::OptionCategory MyToolCategory("my-tool options");
        clang::tooling::CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
        clang::tooling::CompilationDatabase &cdb = OptionsParser.getCompilations();
        // qDebug()<<"cmd from op ..."<<cdb.getCompileCommands("abc.c").c_str();

        auto cmds = cdb.getCompileCommands("demos/rtc_code1.cpp");
        std::for_each(cmds.begin(), cmds.end(), [](clang::tooling::CompileCommand &cmd) {
                // qDebug()<<cmd.CommandLine().c_str();
                auto lines = cmd.CommandLine;
                std::for_each(lines.begin(), lines.end(), [](std::string line) {
                        qDebug()<<"hehe,,,"<<line.c_str();
                    });
            });

        clang::tooling::ClangTool Tool(OptionsParser.getCompilations(),
                                       OptionsParser.getSourcePathList());

    
        // Tool.run(clang::tooling::newFrontendActionFactory<clang::DumpTokensAction>().get());
        // Tool.run(clang::tooling::newFrontendActionFactory<clang::DumpRawTokensAction>().get());

        qDebug()<<"\n\nmodule info...";
        Tool.run(clang::tooling::newFrontendActionFactory<clang::DumpModuleInfoAction>().get());
        // Tool.run(clang::tooling::ToolAction *Action);

        qDebug()<<"gen mod...";
        // Tool.run(clang::tooling::newFrontendActionFactory<clang::GenerateModuleAction>().get());

        qDebug()<<"cgllvm...";
        // bool bret = Tool.run(clang::tooling::newFrontendActionFactory<clang::EmitLLVMOnlyAction>().get());
        clang::EmitLLVMOnlyAction llvm_act;
        // bool bret = ci.ExecuteAction(llvm_act);
        qDebug()<<"cgllvm done...";


        ci.createDiagnostics();
    
        std::string ModuleName = "abcdefg";
    
        clang::CodeGenOptions CGOptions = ci.getCodeGenOpts();
        clang::TargetOptions to = ci.getTargetOpts();

        llvm::LLVMContext &ctx = llvm::getGlobalContext();
    
        llvm::LTOCodeGenerator *ltocg = new llvm::LTOCodeGenerator();
        // ltocg->setTargetOptions(to);
        llvm::TargetOptions lto;
        lto.PrintMachineCode = true;

        ltocg->setTargetOptions(lto);
        ltocg->setCodeGenDebugOptions("emit-llvm");
        ltocg->parseCodeGenDebugOptions();
    
        std::string errMsg;
        // ltocg->compile(int *length, true, true, true, errMsg)
        ltocg->writeMergedModules("/tmp/hehe.tmp", errMsg);
        qDebug()<<"errMsg:"<<errMsg.c_str();

        // clang::CodeGenerator cgen = clang::CreateLLVMCodeGen(ci.getDiagnostics(), ModuleName, 
        //                                                      CGOptions,
        //                                                      to, ctx);


        // 
        // clang::CreateLLVMCodeGen(clang::DiagnosticsEngine &Diags, 
        // const std::string &ModuleName, const clang::CodeGenOptions &CGO, 
        // const clang::TargetOptions &TO, llvm::LLVMContext &C);
        clang::CompilerInvocation cin;
        bret = clang::CompilerInvocation::CreateFromArgs(cin, &argv[1], &argv[0]+argc, ci.getDiagnostics());
        qDebug()<<"ca inv:"<<bret;
        ci.setInvocation(&cin);

        clang::DumpModuleInfoAction act;
        qDebug()<<"exec:...";
        bool ret = ci.ExecuteAction(llvm_act);
        qDebug()<<"exec:"<<ret;

    }

    return 0;
}













