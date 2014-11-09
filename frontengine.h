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
    class FunctionDecl;
    class TranslationUnitDecl;
    class CXXRecordDecl;
    class CXXMethodDecl;
    class CXXConstructorDecl;
    class ClassTemplateDecl;

    namespace driver {
        class Driver;
    };
};


// 生成与维护动态的Qt类和相关方法和函数的AST
// 这类里应该也不需要Compiler组件了，在确定完全使用预生成的ast文件后。
// 或者是CompilerInstance或者CompilerInvocation能够更可定制化，
// 不直接从文件系统搜索文件，使用PCH，这样也可以考虑更动态创建AST。
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
    clang::ASTContext *mrgctx = NULL;
    clang::ASTImporter *mrgimp;
    clang::TranslationUnitDecl *mtrunit = NULL;
    // 一个来自units中的decl缓存，动态实时维护，提供通过类名查找decl的速度。

    QHash<QString, clang::CXXRecordDecl*> decls;

public:
    FrontEngine();
    ~FrontEngine();

    bool init();
    bool initCompiler();

    clang::ASTContext &getASTContext();
    clang::ASTUnit *getASTUnit() { return mrgunit; }
    
    // 单独的加载ast文件方法，
    bool loadPreparedASTFile();
    // 通过预先生成的AST文件，加载AST工作。
    void dumpast(); // for test

    bool parseHeader();
    // 给定头文件路径构建AST工作。
    bool parseHeader(QString path);
    // 给定一个类名，执行构建AST工作。
    bool parseClass(QString klass);
    // 获取一个方法的参数默认值
    bool get_method_default_args(QString klass, QString method, QString symbol_name,
                                 QVector<QVariant> &dargs);
    // 获取一个方法的参数默认值
    bool get_method_default_args2(QString klass, QString method, QString symbol_name,
                                  QVector<QVariant> &dargs);
    // 通过正在调用的信息类名，方法名和参数，查找匹配的类方法。
    // 并返回方法的完整prototype和mangled symbol名
    // 在这是直接返回clang::Decl*呢，还是返回字符串表示呢？
    bool resolve_symbol(QString klass, QString method, QVector<QVariant> uargs,
                        QString &symbol_name, QString &proto_str);
    // 查找一个方法的返回值类型
    bool get_method_return_type(QString klass, QString method, QVector<QVariant> uargs, 
                                QString mangle_name, QVariant &retype);

private:
public:
    clang::FunctionDecl *find_free_function(QString fname);
    clang::CXXRecordDecl* find_class_decl(QString klass);
    clang::ClassTemplateDecl* find_tpl_class_decl(QString klass);
    QVector<clang::CXXMethodDecl*> find_method_decls(clang::CXXRecordDecl *decl, 
                                                     QString klass, QString method);
    QVector<clang::CXXMethodDecl*> find_tpl_method_decls(clang::ClassTemplateDecl *decl,
                                                                  QString klass, QString method);

    // 查找一个类的符合条件的构造函数定义。
    clang::CXXConstructorDecl* find_ctor_decl(clang::CXXRecordDecl *decl, 
                                              QString klass, QVector<QVariant> uargs);
    // 查找一个类的符合条件的方法定义。
    clang::CXXMethodDecl* find_method_decl(clang::CXXRecordDecl *decl, 
                                           QString klass, QString method, QVector<QVariant> uargs);
    clang::CXXMethodDecl* find_method_decl_from_base(clang::CXXRecordDecl *decl, 
                                                     QString klass, QString method, QVector<QVariant> uargs);
    bool method_match_by_uargs(clang::CXXMethodDecl *decl, 
                               QString klass, QString method, QVector<QVariant> uargs);
    bool mangle_method_to_symbol(clang::CXXMethodDecl *decl, 
                                 QString &symbol_name, QString &proto_str);
    // 获取方法的默认参数值
    bool get_method_default_params(clang::CXXMethodDecl *decl, QVector<QVariant> &dparams);
    // 获取一个方法的返回值类型，现在以字符串格式表示，后续可以考虑使用类型标识。
    QVariant get_method_return_type(clang::CXXMethodDecl *decl);

    // 查找类的enum定义
    int get_class_enum(clang::CXXRecordDecl *decl, QString enum_name);
    // 查找Qt namespace的enum定义
    int get_qtns_enum(QString enum_name);
};

#endif /* FRONTENGINE_H */

















