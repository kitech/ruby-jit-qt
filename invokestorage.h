#ifndef INVOKESTORAGE_H
#define INVOKESTORAGE_H

#include <iostream>
#include <QString>
#include <QChar>

constexpr int MAX_IS_COUNT = 10;

class InvokeStorage2 {
public:
    InvokeStorage2(){}
    ~InvokeStorage2(){}

public:
    int iretval;
    bool bretval;
    QString sretval;
    QChar cretval;

    QString *sbyvalret; // by value return result
    void *vbyvalret;

    // 最大支持10个参数，参数的临时值放在这
    int ival[10];
    bool bval[10];
    QString sval[10];
    QChar cval[10];
    void *vval[10]; // for class ABC *
    // std::string cxxsval[10]; // why this cause crash???
    char csval[10][256];
} ;
// TODO 多线程支持
// InvokeStorage2 gis;



#endif /* INVOKESTORAGE_H */
