#ifndef COMPILERENGINE_H
#define COMPILERENGINE_H

/*
  用于实时编译需要执行的代码段，生成ll代码
 */

#include <QtCore>

namespace clang {
    class Decl;
    class CXXRecordDecl;
    class CXXMethodDecl;
    class ASTContext;
    class CompilerInstance;
    class CompilerInvocation;
    class ASTUnit;
    namespace driver {
        class Driver;
    };
};

class CompilerEngine
{
public:
    CompilerEngine();
    ~CompilerEngine();

public:
    bool initCompiler();
    bool tryCompile(clang::CXXRecordDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit);
    bool tryCompile2(clang::CXXRecordDecl *decl, clang::ASTContext &ctx);
    bool tryCompile3(clang::CXXRecordDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit);

public:
    clang::CompilerInstance *mcis = NULL;
    clang::CompilerInvocation *mciv = NULL;
    clang::driver::Driver *mdrv = NULL;
};


#endif /* COMPILERENGINE_H */
