#ifndef FINDSYMBOLVISITOR_H
#define FINDSYMBOLVISITOR_H

#include <clang/AST/StmtVisitor.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <QtCore>

namespace clang {
    class CXXTemporaryObjectExpr;
    class CXXDefaultArgExpr;
    class CXXConstructExpr;
    class CXXThisExpr;
    class CXXMemberCallExpr;
    class FunctionDecl;
}

class CompilerEngine;
class CompilerUnit;

class FindSymbolVisitor : public clang::RecursiveASTVisitor<FindSymbolVisitor>
{
public:
    clang::Decl *mdecl = NULL;
    QHash<QString, clang::FunctionDecl*> mSymbols;
    CompilerEngine *mce = NULL;
    CompilerUnit *mcu = NULL;
    
    FindSymbolVisitor(clang::Decl *d, CompilerEngine *ce, CompilerUnit *cu);
    ~FindSymbolVisitor() {}

public:
    void Run();
    void Run(clang::Decl *d);
    // bool VisitExpr(clang::Expr *ce);
    // bool VisitCallExpr(clang::CallExpr *ce);
    // bool VisitCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr *ce);
    // bool VisitCXXConstructExpr(clang::CXXConstructExpr *ce);
    // bool VisitCXXDefaultArgExpr(clang::CXXDefaultArgExpr *ce);

    bool TraverseExpr(clang::Expr *ce);
    bool TraverseCallExpr(clang::CallExpr *ce);
    bool TraverseCXXMemberCallExpr(clang::CXXMemberCallExpr *ce);
    bool TraverseCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr *ce);
    bool TraverseCXXConstructExpr(clang::CXXConstructExpr *ce);
    bool TraverseCXXCtorInitializer(clang::CXXCtorInitializer *ce);
    bool TraverseCXXDefaultArgExpr(clang::CXXDefaultArgExpr *ce);
    bool TraverseCXXThisExpr(clang::CXXThisExpr *ce);
    bool TraverseDeclRefExpr(clang::DeclRefExpr *ce);
    bool TraverseImplicitCastExpr(clang::ImplicitCastExpr *ce);
    bool TraverseMemberExpr(clang::MemberExpr *ce);
    bool TraverseMaterializeTemporaryExpr(clang::MaterializeTemporaryExpr *ce);

    bool shouldVisitTemplateInstantiations() const { return true; }
    bool shouldVisitImplicitCode() const { return true; }
};

/*
class FindSymbolVisitor : public clang::StmtVisitor<FindSymbolVisitor, bool>
{
public:
    clang::Decl *mdecl = NULL;
    FindSymbolVisitor(clang::Decl *d);
    ~FindSymbolVisitor();

public:
    void Run();
    bool VisitExpr(clang::Expr *ce);
    bool VisitCallExpr(clang::CallExpr *ce);
    bool VisitCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr *ce);
    bool VisitCXXConstructExpr(clang::CXXConstructExpr *ce);
    bool VisitCXXDefaultArgExpr(clang::CXXDefaultArgExpr *ce);
};
*/

#endif /* FINDSYMBOLVISITOR_H */
