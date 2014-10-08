#ifndef FRONTENGINE_H
#define FRONTENGINE_H

#include <QtCore>

/*
  用于解析头文件，实时提供类的声明原型，类方法的返回值，类方法的默认参数值。
 */



namespace clang {
    class ASTContext;
    class CompilerInstance;
    class CompilerInvocation;
    class ASTUnit;
    class ASTImporter;
    class MangleContext;

    namespace driver {
        class Driver;
    };
};

class FrontEngine
{
public:
    clang::ASTContext *astctx;
    clang::CompilerInstance *mcis;
    clang::CompilerInvocation *mciv;
    clang::driver::Driver *mdrv;
    clang::MangleContext *mgctx;
public:
    QHash<QString, clang::ASTUnit*> units;
    clang::ASTUnit *mrgunit;
    clang::ASTContext *mrgctx;
    clang::ASTImporter *mrgimp;
    
public:
    FrontEngine();
    ~FrontEngine();

    bool init();
    bool initCompiler();

    bool parseHeader();
    bool parseHeader(QString path);
    bool parseClass(QString klass);
    bool get_method_default_args(QString klass, QString method, QString symbol_name
                                , QVector<QVariant> &dargs);
};

#endif /* FRONTENGINE_H */










