
#include <clang/AST/ASTContext.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/DeclCXX.h>

#include <QtCore>

#include "compilerengine.h"
#include "ivm/dbghelper.h"
#include "findsymbolvisitor.h"


// 实现Visitor的正确姿势是啥呢？总感觉现在写的有点不对劲。
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
    /*
    // for ctor's CtorInit
    // TODO should move to TraverseCXXConstructExpr()
    if (llvm::isa<clang::CXXConstructorDecl>(d)) {
        auto ctor_decl = llvm::cast<clang::CXXConstructorDecl>(d);
        for (auto ci: ctor_decl->inits()) {
            auto e = ci->getInit();
            qDebug()<<"visit ctor init:"<<e<<e->getStmtClassName();
            this->TraverseStmt(e);
        }

        // parameters
        for (auto pd: ctor_decl->params()) {
            qDebug()<<"param:"<<pd<<pd->getNameAsString().c_str();
            qDebug()<<"param:"<<pd<<pd->hasInit();
            if (pd->hasInit()) {
                qDebug()<<"param:"<<pd->getInit()->getStmtClassName();
                this->TraverseStmt(pd->getInit());
            }
        }
    }

    clang::Stmt *stmts = d->getBody();
    if (stmts != NULL) {
        // qDebug()<<"Visiting stmts:"<<d;
        this->TraverseStmt(stmts);
    } else {
        // qDebug()<<"Null stmts:"<<d;
    }
    */
    this->TraverseDecl(d);
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
    for (clang::Stmt *se: ce->children()) {
        this->TraverseStmt(se);
    }
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
    clang::CXXConstructorDecl *ctor = ce->getConstructor();
    auto mgctx = mdecl->getASTContext().createMangleContext();
    std::string str; llvm::raw_string_ostream stm(str);
    mgctx->mangleName(clang::cast<clang::NamedDecl>(ctor), stm);
    qDebug()<<"mangle name:"<<stm.str().c_str();
    mSymbols[QString::fromStdString(stm.str())] = llvm::cast<clang::FunctionDecl>(ctor);

    if (ctor->isTemplateInstantiation()) {
        qDebug()<<"222:"<<QDateTime::currentDateTime();    
        // this->mce->instantiate_method(this->mcu, llvm::cast<clang::CXXMethodDecl>(fd));
        qDebug()<<"222:"<<QDateTime::currentDateTime();
    }
    for (auto ch: ce->children()) {
        // qDebug()<<ch<<ch->getStmtClassName();
    }
    this->TraverseExpr(ce);
    DUMP_COLOR(ce->getConstructor());
    this->Run(ce->getConstructor());
    return true;
}

bool FindSymbolVisitor::TraverseCXXCtorInitializer(clang::CXXCtorInitializer *ce)
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

bool FindSymbolVisitor::TraverseDeclRefExpr(clang::DeclRefExpr *ce)
{
    qDebug()<<ce;
    auto mgctx = mdecl->getASTContext().createMangleContext();
    std::string str; llvm::raw_string_ostream stm(str);
    // mgctx->mangleName(clang::cast<clang::NamedDecl>(ce->getFoundDecl()), stm);
    // qDebug()<<"mangle name:"<<stm.str().c_str();
    qDebug()<<"decl name:"<<ce->getDecl()->getDeclName().getAsString().c_str();
 
    return true;
}

bool FindSymbolVisitor::TraverseImplicitCastExpr(clang::ImplicitCastExpr *ce)
{
    qDebug()<<ce;
    clang::Expr *se = ce->getSubExpr();
    // qDebug()<<se->getStmtClassName();
    this->TraverseExpr(se);
    return true;
}

bool FindSymbolVisitor::TraverseMemberExpr(clang::MemberExpr *ce)
{
    qDebug()<<ce;
    clang::ValueDecl *vd = ce->getMemberDecl();
    qDebug()<<"member name:"<<vd->getDeclName().getAsString().c_str();
    auto mgctx = mdecl->getASTContext().createMangleContext();
    std::string str; llvm::raw_string_ostream stm(str);
    mgctx->mangleName(clang::cast<clang::NamedDecl>(ce->getMemberDecl()), stm);
    qDebug()<<"mangle name:"<<stm.str().c_str();
    return true;
}

bool FindSymbolVisitor::TraverseMaterializeTemporaryExpr(clang::MaterializeTemporaryExpr *ce)
{
    qDebug()<<ce;
    this->TraverseExpr(ce);
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
