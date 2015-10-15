
#include "namelookup.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
// 需要把Sema.h中的emit()函数改名，否则会与Qt中的emit冲突。
#include <clang/Sema/Sema.h>
#include <clang/Sema/Lookup.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Basic/IdentifierTable.h>

#include "frontengine.h"

bool test_code()
{
    // 这个函数生成的II能否用于namelookup呢？
    auto str2ii = [](QString name) -> clang::IdentifierInfo * {
        std::string xname = name.toStdString();
        llvm::StringRef rname(xname);

        clang::LangOptions opts;
        auto *idt = new clang::IdentifierTable(opts);
        auto &II = idt->getOwn(rname);

        llvm::StringRef sname = II.getName();
        qDebug()<<sname.str().c_str();
        idt->PrintStats();

        return &II; // unsafe
        return NULL;
    };

    auto str2dn = [&](QString name) -> clang::DeclarationName {
        clang::IdentifierInfo *II = NULL;

        II = str2ii(name);
        assert(II->getName().str() == name.toStdString());
        clang::DeclarationName dn(II);
        return dn;
    };
    
    return true;
}

bool nlu_find_method(FrontEngine *fe, QString method, clang::CXXRecordDecl *rd)
{
    auto &ctx = fe->getASTContext();
    auto unit = fe->getASTUnit();
    auto &sema = unit->getSema();    

    
    //
    method = "arg";
    
    clang::NamedDecl *nd = NULL;
    for (auto md: rd->methods()) {
        if (llvm::isa<clang::NamedDecl>(md)) {
            if (method == md->getName().data()) {
                nd = md;
                break;
            }
        }
    }
    qDebug()<<nd;
    // nd->dumpColor();
    
    clang::DeclarationName dname(nd->getIdentifier());
    clang::LookupResult lur(sema, dname, clang::SourceLocation(),
                            clang::Sema::LookupMemberName);
    clang::DeclContext *dctx = clang::Decl::castToDeclContext(rd);
    bool bret = sema.LookupQualifiedName(lur, dctx);
    qDebug()<<bret<<dctx->getDeclKindName()
            <<lur.getResultKind();
    // dname.dump();
    int idx = 0;
    for (auto it = lur.begin(); it != lur.end(); it++, idx++) {
        auto td = *it;
        qDebug()<<"idx="<<idx;
        // td->dumpColor();
    }
    qDebug()<<"idx="<<idx;

    //
    QVariant vv(idx);
    llvm::APInt iv(sizeof(int), vv.toInt());
    clang::Expr *te = clang::IntegerLiteral::Create(ctx, clang::Stmt::EmptyShell());
    clang::ADLResult adlur;
    std::vector<clang::Expr*> args = {te};

    qDebug()<<"......";
    // sema.ArgumentDependentLookup(dname, clang::SourceLocation(), args, adlur);
    qDebug()<<"......";    
    idx = 0;
    for (auto it = adlur.begin(); it != adlur.end(); it++, idx++) {
        
    }
    qDebug()<<idx;

    idx = 0;
    for (auto it = lur.begin(); it != lur.end(); it++, idx++) {
        auto td = *it;
        // td->dumpColor();
        auto ea = sema.CheckEnableIf(llvm::cast<clang::FunctionDecl>(td), args, true);
        // qDebug()<<"idx="<<idx<<ea;
    }
    qDebug()<<"idx="<<idx;
    
    return false;
}
