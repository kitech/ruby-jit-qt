#include "fix_clang_undef_eai.h"

#include <QDebug>
#include <QVariant>

#include <llvm/IR/Value.h>
#include <clang/AST/Expr.h>

#include "invokestorage.h"

int EvalType::id = qMetaTypeId<EvalType>();
EvalType::EvalType(clang::Expr *e, llvm::Value *v)
    : ve(e), vv(v)
{
}

void is_test1()
{
    EvalType v(0, 0);
    QVariant a = QVariant::fromValue(v);
    qDebug()<<a;
    EvalType v2 = a.value<EvalType>();
    qMetaTypeId<EvalType>();
    int i = 5;
    int id = qMetaTypeId<EvalType>();
    switch (i) {
    case 1:break;
        // case EvalType::id:  // 
        break;
    };
}












