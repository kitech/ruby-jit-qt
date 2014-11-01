#ifndef INVOKESTORAGE_H
#define INVOKESTORAGE_H

#include <iostream>
#include <QString>
#include <QChar>

constexpr int MAX_IS_COUNT = 10;

class InvokeStorage2 {
public:
    InvokeStorage2(){
        pcsval = (char**)calloc(sizeof(char*), MAX_IS_COUNT);
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
} ;
// TODO 多线程支持
// TODO 更节省内存的存储方式
// InvokeStorage2 gis;



#endif /* INVOKESTORAGE_H */










