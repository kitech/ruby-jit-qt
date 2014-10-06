#include <QtCore>

#include "llvm/Support/Host.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/ModuleLoader.h"

////////


static void get_class_method(QString classname)
{
    clang::CompilerInstance ci;
    ci.createDiagnostics();

    ci.createFileManager();
    // clang::SourceManager sm = ci.createSourceManager();
    // llvm::IntrusiveRefCntPtr<clang::TargetOptions> pto(new clang::TargetOptions());
    // clang::ASTContext actx = ci.createASTContext();

    using namespace clang;
    using namespace clang::ast_matchers;

    StatementMatcher LoopMatcher =
        forStmt(hasLoopInit(declStmt(hasSingleDecl(varDecl(
            hasInitializer(integerLiteral(equals(0)))))))).bind("forLoop");

    class LoopPrinter : public MatchFinder::MatchCallback {
    public :
        virtual void run(const MatchFinder::MatchResult &Result) {
            if (const ForStmt *FS = Result.Nodes.getNodeAs<clang::ForStmt>("forLoop"))
                FS->dump();
        }
    };

    // OKed
    DeclarationMatcher MethodMatcher = 
        // classTemplateDecl(methodDecl(ofClass(hasName("QString"))))
        methodDecl(ofClass(hasName("QString")))
        .bind("matchMethod");

    class MethodPrinter : public MatchFinder::MatchCallback {
    public :
        virtual void run(const MatchFinder::MatchResult &Result) {
            auto nodes = Result.Nodes.getMap();
            foreach (auto node, nodes) {
                qDebug()<<node.first.c_str()<<nodes.size();
            }
            const CXXMethodDecl *st = Result.Nodes.getNodeAs<CXXMethodDecl>("matchMethod");
            qDebug()<<st << st->getNameInfo().getAsString().c_str();
            // st->dump();
            
        }
    };


    // 注，namespace名字全是小写的，类名首字母全是大写的
    const char *argv[] = {
        "anonc", "/usr/include/qt/QtCore/qstring.h", "--",
        // "-Xclang", "-ast-dump", "-fsyntax-only", 
        "-x", "c++",
        "-I/usr/include/qt", "-fPIC",
        "-I/usr/lib/clang/3.5.0/include/"
    };
    int argc = 8;

    static llvm::cl::OptionCategory MyToolCategory("my-tool options");
    clang::tooling::CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);

    clang::tooling::ClangTool Tool(OptionsParser.getCompilations(),
                                   OptionsParser.getSourcePathList());


    LoopPrinter Printer;
    MethodPrinter mPrinter;
    MatchFinder Finder;
    // Finder.addMatcher(LoopMatcher, &Printer);
    Finder.addMatcher(MethodMatcher, &mPrinter);

    Tool.run(clang::tooling::newFrontendActionFactory<>(&Finder).get());
    // Tool.run(clang::tooling::newFrontendActionFactory<clang::SyntaxOnlyAction>().get());
    
}

int main(int argc, char **argv)
{
    QString klass_name = "QString";
    get_class_method(klass_name);
    return 0;
}
