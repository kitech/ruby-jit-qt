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
    // 一个来自units中的decl缓存，动态实时维护，提供通过类名查找decl的速度。
    QHash<QString, clang::CXXRecordDecl*> decls;

public:
    FrontEngine();
    ~FrontEngine();

    bool init();
    bool initCompiler();

    // 通过预先生成的AST文件，加载AST工作。
    bool parseHeader();
    // 给定头文件路径构建AST工作。
    bool parseHeader(QString path);
    // 给定一个类名，执行构建AST工作。
    bool parseClass(QString klass);
    // 获取一个方法的参数默认值
    bool get_method_default_args(QString klass, QString method, QString symbol_name,
                                 QVector<QVariant> &dargs);
    // 通过正在调用的信息类名，方法名和参数，查找匹配的类方法。
    // 并返回方法的完整prototype和mangled symbol名
    // 在这是直接返回clang::Decl*呢，还是返回字符串表示呢？
    bool symbol_resolve(QString klass, QString method, QVector<QVariant> uargs,
                        QString &symbol_name, QString &proto_str);

};

#endif /* FRONTENGINE_H */

















