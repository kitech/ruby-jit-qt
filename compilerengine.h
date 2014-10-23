#ifndef COMPILERENGINE_H
#define COMPILERENGINE_H

/*
  用于实时编译需要执行的代码段，生成ll代码
 */

#include <QtCore>

namespace llvm {
    class Module;
};

namespace clang {
    class Decl;
    class CXXRecordDecl;
    class CXXMethodDecl;
    class CXXConstructorDecl;
    class ClassTemplateDecl;
    class ASTContext;
    class CompilerInstance;
    class CompilerInvocation;
    class ASTUnit;
    namespace driver {
        class Driver;
    };
    namespace CodeGen {
        class CodeGenModule;
    };
};

class CompilerEngine
{
public:
    CompilerEngine();
    ~CompilerEngine();

public:
    llvm::Module* conv_ctor(clang::ASTContext &ctx, clang::CXXConstructorDecl *ctor);
    llvm::Module* conv_method(clang::ASTContext &ctx, clang::CXXMethodDecl *mth);
    // bool check_inline_symbol();
    QString mangle_ctor(clang::ASTContext &ctx, clang::CXXConstructorDecl *ctor);
    QString mangle_method(clang::ASTContext &ctx, clang::CXXMethodDecl *ctor);

    // 第一种方式，拿到还未定义的symbol，到ast中查找
    // 第二种方式，解析C++方法的源代码，找到这个未定义的symbol，从decl再把它编译成ll，效率更好。
    void decl2def(llvm::Module *mod, clang::ASTContext &ctx,
                  clang::CodeGen::CodeGenModule &cgmod, 
                  clang::Decl *decl, int level, QHash<QString, bool> noinlined);

public:
    bool initCompiler();
    bool tryCompile(clang::CXXRecordDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit);
    bool tryCompile2(clang::CXXRecordDecl *decl, clang::ASTContext &ctx);
    bool tryCompile3(clang::CXXRecordDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit);
    bool tryCompile4(clang::CXXRecordDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit);
    bool tryCompile_tpl(clang::ClassTemplateDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit);

public:
    clang::CompilerInstance *mcis = NULL;
    clang::CompilerInvocation *mciv = NULL;
    clang::driver::Driver *mdrv = NULL;
    llvm::Module *mtmod = NULL;
};


#endif /* COMPILERENGINE_H */





