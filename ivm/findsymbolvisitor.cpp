
#include <clang/AST/ASTContext.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/DeclCXX.h>

#include <QtCore>

#include "compilerengine.h"
#include "ivm/dbghelper.h"
#include "findsymbolvisitor.h"



///////////////
FindSymbolVisitor::FindSymbolVisitor(clang::Decl *d, CompilerEngine *ce, CompilerUnit *cu)
    : mdecl(d), mce(ce), mcu(cu)
{}

void FindSymbolVisitor::Run()
{
    this->Run(mdecl);
}

void FindSymbolVisitor::Run(clang::Decl *d)
{
    clang::Stmt *stmts = d->getBody();
    if (stmts != NULL) {
        // qDebug()<<"Visiting stmts:"<<d;
        this->TraverseStmt(stmts);
    } else {
        // qDebug()<<"Null stmts:"<<d;
    }

    // for ctor's CtorInit
    if (llvm::isa<clang::CXXConstructorDecl>(d)) {
        auto ctor_decl = llvm::cast<clang::CXXConstructorDecl>(d);
        for (auto ci: ctor_decl->inits()) {
            auto e = ci->getInit();
            // qDebug()<<"visit ctor init:"<<e;
            this->TraverseStmt(e);
        }
    }
    // this->TraverseDecl(d);
}

/*
bool FindSymbolVisitor::VisitExpr(clang::Expr *ce)
{
    // qDebug()<<ce<<ce->getStmtClassName();
    return true;
}

bool FindSymbolVisitor::VisitCallExpr(clang::CallExpr *ce)
{
    // qDebug()<<ce;
    auto mgctx = mdecl->getASTContext().createMangleContext();
    std::string str; llvm::raw_string_ostream stm(str);
    mgctx->mangleName(clang::cast<clang::NamedDecl>(ce->getCalleeDecl()), stm);
    qDebug()<<"mangle name:"<<stm.str().c_str();
    
    return true;
}

bool FindSymbolVisitor::VisitCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr *ce)
{
    qDebug()<<ce;
    return true;
}

bool FindSymbolVisitor::VisitCXXConstructExpr(clang::CXXConstructExpr *ce)
    {
    qDebug()<<ce;
    return true;
}

bool FindSymbolVisitor::VisitCXXDefaultArgExpr(clang::CXXDefaultArgExpr *ce)
{
    qDebug()<<ce;
    return true;
}
*/

/////////
bool FindSymbolVisitor::TraverseExpr(clang::Expr *ce)
{
    qDebug()<<ce;
    return true;
}

bool FindSymbolVisitor::TraverseCallExpr(clang::CallExpr *ce)
{
    qDebug()<<ce;
    auto mgctx = mdecl->getASTContext().createMangleContext();
    std::string str; llvm::raw_string_ostream stm(str);
    mgctx->mangleName(clang::cast<clang::NamedDecl>(ce->getCalleeDecl()), stm);
    qDebug()<<"mangle name:"<<stm.str().c_str();
    mSymbols[QString::fromStdString(stm.str())] = llvm::cast<clang::FunctionDecl>(ce->getCalleeDecl());

    clang::FunctionDecl *fd = llvm::cast<clang::FunctionDecl>(ce->getCalleeDecl());
    if (fd->isTemplateInstantiation()) {
        qDebug()<<"222:"<<QDateTime::currentDateTime();    
        this->mce->instantiate_method(this->mcu, llvm::cast<clang::CXXMethodDecl>(fd));
        qDebug()<<"222:"<<QDateTime::currentDateTime();
    }
    
    DUMP_COLOR(ce->getCalleeDecl());
    this->Run(ce->getCalleeDecl());
    return true;
}
bool FindSymbolVisitor::TraverseCXXMemberCallExpr(clang::CXXMemberCallExpr *ce)
{    
    qDebug()<<ce;
    auto mgctx = mdecl->getASTContext().createMangleContext();
    std::string str; llvm::raw_string_ostream stm(str);
    mgctx->mangleName(clang::cast<clang::NamedDecl>(ce->getCalleeDecl()), stm);
    qDebug()<<"mangle name:"<<stm.str().c_str();
    mSymbols[QString::fromStdString(stm.str())] = llvm::cast<clang::FunctionDecl>(ce->getCalleeDecl());

    clang::FunctionDecl *fd = llvm::cast<clang::FunctionDecl>(ce->getCalleeDecl());
    if (fd->isTemplateInstantiation()) {
        qDebug()<<"222:"<<QDateTime::currentDateTime();            
        this->mce->instantiate_method(this->mcu, llvm::cast<clang::CXXMethodDecl>(fd));
        qDebug()<<"222:"<<QDateTime::currentDateTime();    
    }

    DUMP_COLOR(ce->getCalleeDecl());    
    this->Run(ce->getCalleeDecl());
    return true;
}

bool FindSymbolVisitor::TraverseCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr *ce)
{
    qDebug()<<ce;
    return true;
}

bool FindSymbolVisitor::TraverseCXXConstructExpr(clang::CXXConstructExpr *ce)
{
    qDebug()<<ce;
    return true;
}

bool FindSymbolVisitor::TraverseCXXDefaultArgExpr(clang::CXXDefaultArgExpr *ce)
{
    qDebug()<<ce;
    return true;
}

bool FindSymbolVisitor::TraverseCXXThisExpr(clang::CXXThisExpr *ce)
{
    qDebug()<<ce;
    return true;
}

//////////////////////
/*

FindSymbolVisitor::FindSymbolVisitor(clang::Decl*d)
    : mdecl(d)
{
}

FindSymbolVisitor::~FindSymbolVisitor()
{
}

void FindSymbolVisitor::Run()
{
    clang::Stmt *stmts = mdecl->getBody();
    if (stmts != NULL) {
        // qDebug()<<"Visiting stmts:"<<mdecl;
        this->Visit(stmts);
    } else {
        // qDebug()<<"Null stmts:"<<mdecl;
    }

    // for ctor's CtorInit
    if (llvm::isa<clang::CXXConstructorDecl>(mdecl)) {
        auto ctor_decl = llvm::cast<clang::CXXConstructorDecl>(mdecl);
        for (auto ci: ctor_decl->inits()) {
            auto e = ci->getInit();
            // qDebug()<<"visit ctor init:"<<e;
            this->Visit(e);
        }
    }    
}

bool FindSymbolVisitor::VisitExpr(clang::Expr *ce)
{
    qDebug()<<ce<<ce->getStmtClassName();
    for (clang::Stmt::child_range I = ce->children(); I; ++I)
        this->Visit(*I);

    return false;
}

bool FindSymbolVisitor::VisitCallExpr(clang::CallExpr *ce)
{
    qDebug()<<ce;
    return false;
}

bool FindSymbolVisitor::VisitCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr *ce)
{
    qDebug()<<ce;
    return false;
}

bool FindSymbolVisitor::VisitCXXConstructExpr(clang::CXXConstructExpr *ce)
{
    qDebug()<<ce;
    return false;
}

bool FindSymbolVisitor::VisitCXXDefaultArgExpr(clang::CXXDefaultArgExpr *ce)
{
    qDebug()<<ce;
    return false;
}

*/
