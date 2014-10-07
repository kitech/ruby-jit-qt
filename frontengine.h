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

public:
    QHash<QString, clang::ASTUnit*> units;
    
public:
    FrontEngine();
    ~FrontEngine();

    bool init();
    bool initCompiler();

    bool parseHeader(QString path);
};

#endif /* FRONTENGINE_H */
