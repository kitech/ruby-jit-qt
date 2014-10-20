
// usage: 在其他位置直接插入#include代码。
// #include "tests.cpp"

#include <QtCore>

#include "tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <cxxabi.h>

#include "compilerengine.h"


int test_demangle() 
{
    auto demangle = [] (QString name) -> QString {
        const char * mangled_name = "_ZN7QString6appendE5QChar";
        mangled_name = name.toLatin1().data();
        mangled_name = "_ZN17QSignalTransition11qt_metacallEN11QMetaObject4CallEiPPv";
        size_t dm_len = 256; // 256 is maybe a good choise
        char *demangled_name = (char*)calloc(1, dm_len);
        int dm_status = 0;

        __cxxabiv1::__cxa_demangle(mangled_name, demangled_name, &dm_len, &dm_status);
        printf("mgn:%s,len:%d,st:%d\n", demangled_name, dm_len, dm_status);

        return QString();
    };

    demangle("");
    return 0;
}

void test_fe()
{
    QVector<QVariant> dargs;
    bool bret;
    FrontEngine *fe = new FrontEngine();
    fe->parseHeader("/usr/include/qt/QtCore/qstring.h");
    // fe->parseHeader("/usr/include/qt/QtWidgets/qwidget.h");
    // fe->parseHeader("/usr/include/qt/QtWidgets/qapplication.h");

    bret = fe->get_method_default_args("QString", "lastIndexOf", 
                                      "_ZNK9YaQString11lastIndexOfE5QChariN2Qt15CaseSensitivityE", dargs);
    bret = fe->get_method_default_args("QString", "lastIndexOf", 
                                      "_ZNK9YaQString11lastIndexOfERK10QStringRefiN2Qt15CaseSensitivityE", dargs);
    bret = fe->get_method_default_args("QString", "lastIndexOf", 
                                      "_ZNK9YaQString11lastIndexOfERK18QRegularExpressioni", dargs);
    bret = fe->get_method_default_args("QString", "lastIndexOf", 
                                      "_ZNK9YaQString11lastIndexOfE13QLatin1StringiN2Qt15CaseSensitivityE", dargs);
    bret = fe->get_method_default_args("QString", "arg", 
                                      "_ZNK9YaQString3argERK7QStringS2_S2_S2_S2_S2_S2_", dargs);
    bret = fe->get_method_default_args("QString", "arg", 
                                      "_ZNK9YaQString3argERK7QStringi5QChar", dargs);

}

void test_parse_class()
{
    FrontEngine *fe = new FrontEngine();
    fe->parseClass("QString");
    fe->parseClass("QWidget");

    fe->parseClass("QString");
    fe->parseClass("QWidget");

    fe->parseClass("QTcpServer");
}

void test_parse_ast()
{
    FrontEngine *fe = new FrontEngine();
    QDateTime btime = QDateTime::currentDateTime();
    fe->parseHeader();
    QDateTime etime = QDateTime::currentDateTime();
    qDebug()<<"use time:"<<btime.msecsTo(etime);
}

void test_piece_compiler()
{
    FrontEngine *fe = new FrontEngine();
    fe->loadPreparedASTFile();
    CompilerEngine *ce = new CompilerEngine();
    qDebug()<<fe<<ce;

    clang::CXXMethodDecl *decl = 0;
    clang::CXXRecordDecl *recdecl = 0;
    clang::ASTContext &ctx = fe->getASTContext();

    QString klass = "QString";
    QString method = "length";
    
    recdecl = fe->find_class_decl(klass);
    QVector<clang::CXXMethodDecl*> mthdecls = fe->find_method_decls(recdecl, klass, method);
    
    qDebug()<<recdecl<<mthdecls.count();

    // ce->tryCompile(recdecl, ctx, fe->mrgunit);
    // ce->tryCompile2(recdecl, ctx);
    // ce->tryCompile3(recdecl, ctx, fe->mrgunit);
    ce->tryCompile4(recdecl, ctx, fe->mrgunit);
}

void test_tpl_piece_compiler()
{
    FrontEngine *fe = new FrontEngine();
    fe->loadPreparedASTFile();
    CompilerEngine *ce = new CompilerEngine();
    qDebug()<<fe<<ce;

    clang::CXXMethodDecl *decl = 0;
    clang::ClassTemplateDecl *tpldecl = 0;
    clang::ASTContext &ctx = fe->getASTContext();

    QString klass = "QTypedArrayData";
    QString method = "sharedNull";
    
    tpldecl = fe->find_tpl_class_decl(klass);
    QVector<clang::CXXMethodDecl*> mthdecls = fe->find_tpl_method_decls(tpldecl, klass, method);
    
    qDebug()<<tpldecl<<mthdecls.count();

    // ce->tryCompile(recdecl, ctx, fe->mrgunit);
    // ce->tryCompile2(recdecl, ctx);
    // ce->tryCompile3(recdecl, ctx, fe->mrgunit);

    ce->tryCompile_tpl(tpldecl, ctx, fe->mrgunit);
}

void test_dumpast()
{
    FrontEngine *fe = new FrontEngine();
    fe->loadPreparedASTFile();
    fe->dumpast();
}



void test_one()
{
    // test_fe();
    // test_piece_compiler();
    // test_dumpast();
    test_tpl_piece_compiler();
}
