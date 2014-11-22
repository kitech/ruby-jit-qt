#ifndef INVOKESTORAGE_H
#define INVOKESTORAGE_H

#include <iostream>
#include <QString>
#include <QChar>
#include <QMetaType>
#include <QStringList>

namespace llvm {
    class Value;
};
namespace clang {
    class Expr;
};

// for QFlag in QVariant
struct xQFlag
{
    union {
        char m[sizeof(QFlag)];
        int i;
    } v;
};

Q_DECLARE_METATYPE(xQFlag);
// METATYPE类型，必须要有一个默认构造函数
struct EvalType {
    EvalType() {}
    EvalType(clang::Expr *e, llvm::Value *v);    
    clang::Expr *ve = NULL; // value expr
    llvm::Value *vv = NULL; // value value
    static int id;
    void *vx = NULL;
    llvm::Value *vf_base = NULL;// Ctor_Base
    QString vf_base_name;
};
Q_DECLARE_METATYPE(EvalType);

constexpr int MAX_IS_COUNT = 10;

class InvokeStorage2 {
public:
    InvokeStorage2(){
        pcsval = (char**)calloc(sizeof(char*), MAX_IS_COUNT);
        fval = (QFlag*)calloc(sizeof(QFlag), MAX_IS_COUNT);
        for (auto i = 0; i < MAX_IS_COUNT; i++) {
            fval[i] = QFlag(0);
            fval[i] = QFlag(1);
        }
    }
    ~InvokeStorage2(){}

public:
    // 参数的返回值临时存储
    int iretval;
    bool bretval;
    QString sretval;
    QChar cretval;

    // 返回值为对象值时的临时对象指针，如QString非QString &，QString*等。
    QString *sbyvalret; // by value return result
    void *vbyvalret;

    // 最大支持10个参数，参数的临时值放在这
    int ival[10];
    long long lval[10];
    bool bval[10];
    QString sval[10];
    QChar cval[10];
    void *vval[10]; // for class ABC *
    // std::string cxxsval[10]; // why this cause crash???
    char csval[10][256];
    char **pcsval;
    QStringList slval[10];
    QFlag *fval;  // flag value
    void *fval2[10]; // flags value
} ;
// TODO 多线程支持
// TODO 更节省内存的存储方式
// InvokeStorage2 gis;



#endif /* INVOKESTORAGE_H */










