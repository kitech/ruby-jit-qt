#ifndef COMPILERENGINE_H
#define COMPILERENGINE_H

/*
  用于实时编译需要执行的代码段，生成ll代码
 */

#include <QtCore>

namespace llvm {
    class LLVMContext;
    class Module;
};

namespace clang {
    class Decl;
    class Stmt;
    class Expr;
    class NamedDecl;
    class FunctionDecl;
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
        class CodeGenFunction;
    };
};

class CompilerUnit;
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
    clang::FunctionDecl* find_callee_decl_by_symbol(clang::Decl *bdecl, QString callee_symbol);
    clang::FunctionDecl* find_callee_decl_by_symbol(clang::Decl *bdecl, QString callee_symbol,
                                                    clang::Stmt *bstmt);
    CompilerUnit *createCompilerUnit(clang::ASTUnit *unit, clang::NamedDecl *decl);
    bool destroyCompilerUnit(CompilerUnit *cu);

    llvm::Module* conv_ctor2(clang::ASTUnit *unit, clang::CXXConstructorDecl *ctor);
    llvm::Module* conv_method2(clang::ASTUnit *unit, clang::CXXMethodDecl *mth);

    bool gen_ctor(CompilerUnit *cu);
    bool gen_method(CompilerUnit *cu, clang::CXXMethodDecl *yamth = NULL);
    bool gen_method_decl(CompilerUnit *cu, clang::CXXMethodDecl *yamth = NULL);
    bool gen_free_function(CompilerUnit *cu, clang::FunctionDecl *yafun = NULL);
    bool gen_undefs(CompilerUnit *cu);
    clang::CXXMethodDecl *get_decl_with_body(clang::CXXMethodDecl *decl);
    bool find_undef_symbols(CompilerUnit *cu);
    bool is_in_type_module(QString symbol);
    bool instantiate_method(CompilerUnit *cu, clang::CXXMethodDecl *tmpl_mthdecl);

public: // test use
    bool initCompiler();
    bool tryCompile(clang::CXXRecordDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit);
    bool tryCompile2(clang::CXXRecordDecl *decl, clang::ASTContext &ctx);
    bool tryCompile3(clang::CXXRecordDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit);
    bool tryCompile4(clang::CXXRecordDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit);
    bool tryCompile_tpl(clang::ClassTemplateDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit);
    bool tryTransform(clang::ClassTemplateDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit,
                      clang::CXXMethodDecl *mth, clang::CXXMethodDecl *mth2);
    bool tryTransform2(clang::ClassTemplateDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit,
                       clang::CXXMethodDecl *mth, clang::CXXMethodDecl *mth2);
    bool tryTransform3(clang::ClassTemplateDecl *decl, clang::ASTContext &ctx, clang::ASTUnit *unit,
                       clang::CXXMethodDecl *mth, clang::CXXMethodDecl *mth2);

public:
    clang::CompilerInstance *mcis = NULL;
    clang::CompilerInvocation *mciv = NULL;
    clang::driver::Driver *mdrv = NULL;
    llvm::Module *mtmod = NULL;
};

class CompilerUnit
{
public:
    clang::CompilerInstance *mcis = NULL;
    clang::ASTUnit *munit = NULL;
    llvm::LLVMContext *mvmctx = NULL;
    llvm::Module *mmod = NULL;
    clang::CodeGen::CodeGenModule *mcgm = NULL;
    clang::CodeGen::CodeGenFunction *mcgf = NULL;
    clang::NamedDecl *mdecl = NULL;
    clang::NamedDecl *mbdecl = NULL; // bodyed
    QVector<QString> mUndefSymbols;
    QHash<QString, bool> mNoinlineSymbols;
};

#endif /* COMPILERENGINE_H */

