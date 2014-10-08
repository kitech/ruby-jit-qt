#include <QtCore>

#include "llvm/Support/Host.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/ModuleLoader.h"


//// default new klass prefix
static const QString klass_prefix = "Ya";

////////
static QString klass_name;  // 要处理的类
static QString module_name; // Qt模块名，如QtCore/QtGui/QtNetwork
static QString file_path;   // 要处理的类所在的头文件，如/path/to/qt/QtCore/qstring.h
static QString gened_code;  // 生成的代码内容
static bool has_meta_object = false; // 判定是否用多继承
static int is_dctor_processed = 0; // fix default ctor lost
static int method_counter = 0;

/////
static int empty_arg_cnter = 0;
static QString empty_arg_prefix = "auto_arg_";

class ParserContext {
public:
    QString klass_name;  // 要处理的类
    QString module_name; // Qt模块名，如QtCore/QtGui/QtNetwork
    QString file_path;   // 要处理的类所在的头文件，如/path/to/qt/QtCore/qstring.h
    QString gened_code;  // 生成的代码内容
    QString gened_cpp_code;
    QString gened_proto_code;  // for metar_protos.cpp
    bool has_meta_object = false; // 判定是否用多继承
    int is_dctor_processed = 0; // fix default ctor lost
    int method_counter = 0;

    int empty_arg_cnter = 0;
    QString empty_arg_prefix = "auto_arg_";

    QString qenums_code;
    QFile hfp;
    QFile cfp;
};
static ParserContext pctx;

/*
  TODO:
  drop ~destructor method or ~destructor empty body ==> Oked 
  drop operatorx() method ==> Oked
  ctor 不能放在public slot:下，但可以使用Q_INVOKABLE。 ==> Oked
  default ctor没翻译过来，被忽略掉了。
  metar 函数不能再重重载了。metaObject。    ==> Oked

  add access type: public|protected|private  ==> Oked
  add arg default value ==> OKed
  add enums
  adjust static method
  some arg has no name  ==> Oked
  multi inherint  ==> Oked
  template class相关类解析不正常，如QMap,QVector,QStack
  template method生成不正常。
  error: call to deleted constructor of
  有些方法返回值丢失了&引用，如QDataStream &writeBytes。 ==> Oked
  signal/slot的特殊处理。
  error: 'QPrivateSignal' is a private member of 'QObject'   ==> Oked
      (是不是所有的信号都有这个类的应用？是的，表示这个方法是一个信号）
      (不过这种信号是私有信号）           
  isdefined检测不准确，不是我希望的那意思。

  现在目标，只生成sharedlib的符号表。
 */
static void parser_decl_for_one(const clang::CXXMethodDecl *decl, const clang::ASTContext &ast_ctx, 
                                const clang::SourceManager &sm)
{
    // test and debug
    qDebug()<<decl << decl->getNameInfo().getAsString().c_str()
            <<"has body:"<<decl->hasBody()
            <<"param size:"<<decl->param_size();

    // decl local vars
    QString tmp;
    QString ctmp;
    QString ptmp;
    QString method_name = QString(decl->getNameInfo().getAsString().c_str());
    QString new_method_name = method_name;
    QString return_type_str = "";


    int drop_cond_count = 0; // 通过解释发现需要跳过的方法
    int is_priv_sig = 0;

    // decl local lambda function
    auto trim_type_class = [] (QString type_str) -> QString {
        QStringList type_elems = type_str.split(" ");
        for (int i = 0; i < type_elems.count(); i ++) {
            if (type_elems.at(i) == "class") {
                type_elems.removeAt(i);
            }
        }

        return type_elems.join(" ");
    };
    auto is_dtor = [&](QString method_name) -> bool { return method_name.startsWith("~"); };
    auto is_ctor = [&](QString method_name) -> bool { return method_name == klass_name; };
    // default ctor
    auto is_dctor = [&](QString method_name, const clang::CXXMethodDecl*decl) -> bool
        { return is_ctor(method_name) && decl->param_size() == 0; };
    // FunctionDecl::isOverloadedOperator()
    auto is_oper = [&](QString method_name) -> bool { return method_name.startsWith("operator"); };
    auto def_arg_string = [&](const clang::Expr *expr) -> QString {
        if (expr == NULL) return QString();
        std::string string_buffer;
        llvm::raw_string_ostream string_stream(string_buffer);
        expr->printPretty(string_stream, nullptr, clang::PrintingPolicy(ast_ctx.getLangOpts()));
        return QString(string_stream.str().c_str());
    };
    auto is_metar = [&](QString method_name) -> bool {
        return method_name == "metaObject"
        || method_name == "qt_metacast" || method_name == "qt_metacall"
        || method_name == "qt_check_for_QOBJECT_macro"
        || method_name == "tr" || method_name == "trUtf8";
    };
    auto is_dptr = [&](QString method_name) -> bool {
        return method_name == "data_ptr";
    };
    auto is_qt_private_signal = [&](const clang::CXXMethodDecl *d) -> bool {
        return d->param_size() == 0 ? false :
        QString(d->getParamDecl(d->param_size()-1)
                ->getOriginalType()
                ->getTypeClassName()).indexOf("QPrivateSignal") >= 0;
    };
    is_priv_sig = is_qt_private_signal(decl);
    auto decl_m2ctor = [&](const clang::CXXMethodDecl *d) -> const clang::CXXConstructorDecl* {
        if (llvm::isa<clang::CXXConstructorDecl>(d))
            return llvm::cast<const clang::CXXConstructorDecl>(d);
        return 0;
    };
    auto is_template_method = [&](const clang::CXXMethodDecl*d) -> bool {
        return d->getDescribedFunctionTemplate();
        // return d->getPrimaryTemplate();
    };
    auto is_qflag_type = [&](clang::QualType t) -> bool {
        return QString(t.getCanonicalType().getAsString().c_str())
        .startsWith("class QFlags<enum ")
        && !QString(t.getCanonicalType().getAsString().c_str())
        .startsWith("class QFlags<enum Qt::");
    };
    auto get_qflag_name = [&](clang::QualType t) -> QString {
        // class QFlags<enum QFileDevice::Permission>
        QRegExp exp("^class QFlags<enum (.*)>$");
        QString str = t.getCanonicalType().getAsString().c_str();
        if (exp.exactMatch(str)) {
            return exp.capturedTexts().at(1) + "s";
        } else {
            return QString();
        }
    };
    auto is_typedef_type = [&](clang::QualType t) -> bool {
        return QString(t->getTypeClassName()) == "Typedef";
    };
    auto is_currclass_typedef = [&](clang::QualType fret_type) -> bool {
        if (clang::isa<clang::TypedefType>(fret_type)) {
            const clang::TypedefType *t = fret_type->getAs<clang::TypedefType>();
            clang::TypedefNameDecl *d = t->getDecl();
            clang::DeclContext *dc = d->getLexicalDeclContext();
            clang::CXXRecordDecl *cxxd = clang::cast<clang::CXXRecordDecl>(dc);
            if (QString(cxxd->getName().data()) == klass_name) return true;
        }
        return false;
    };

    // 
    clang::SourceRange sr = decl->getSourceRange();

    // decl->getNameInfo().getName().dump();
    clang::QualType fret_type = decl->getReturnType();
    clang::QualType res_type = decl->getCallResultType(); // maby lost '&' reference char
    clang::QualType unq_type = fret_type.getUnqualifiedType();
    qDebug()<<"return type:"<< decl->isNoReturn() << res_type->isVoidType()
            << res_type.getAsString().c_str()
            << "decled:"<<decl->isDefined()
            <<"has body:"<<decl->hasBody()
            << "has inline body:"<<decl->hasInlineBody()
            << "fret type:"<<fret_type.getAsString().c_str()
            << "\n"
            << "decl is def?:" << decl->isThisDeclarationADefinition()
            << "is outofline:" <<decl->isOutOfLine()
            << "is template:" << is_template_method(decl)
            << "is template spec:"<<decl->isFunctionTemplateSpecialization()
            << "is tpl inst:"<<decl->isTemplateInstantiation();
    fret_type->dump();
    res_type->dump();
    unq_type->dump();
    qDebug()<<"ret enumt:"<<fret_type->isIntegralOrEnumerationType()
            <<res_type->isIntegralOrEnumerationType();
    qDebug()<<"res enumt:"<<fret_type->isIntegralOrUnscopedEnumerationType()
            <<res_type->isIntegralOrUnscopedEnumerationType();
    qDebug()<<"is enut:"<<fret_type->isEnumeralType()
            <<res_type->isEnumeralType()
            <<fret_type->getTypeClassName()
            <<fret_type.getCanonicalType().getAsString().c_str()
            <<"is typedef:"<<is_typedef_type(fret_type)
            <<llvm::isa<clang::TypedefType>(fret_type);

    fret_type->getCanonicalTypeInternal()->dump();
    fret_type.getCanonicalType()->dump();
    if (is_qflag_type(fret_type)) qDebug()<<get_qflag_name(fret_type);
    if (clang::isa<clang::TypedefType>(fret_type)) {
        const clang::TypedefType *t = fret_type->getAs<clang::TypedefType>();
        clang::TypedefNameDecl *d = t->getDecl();
        clang::DeclContext *dc = d->getLexicalDeclContext();
        // clang::DeclContext *pdc = dc->getParent();
        clang::CXXRecordDecl *cxxd = clang::cast<clang::CXXRecordDecl>(dc);
        qDebug()<<t<<d<<d->isCXXInstanceMember()<<dc->getDeclKindName()
                <<dc->isRecord()<<clang::isa<clang::CXXRecordDecl>(dc)
                <<cxxd->getName().data();
        
        d->dump();
    }

    // 
    // fix some inline function define in *.h.
    // if (decl->isDefined()) {
    //  qDebug()<<"first defined???"<<decl->hasBody()<<decl->hasInlineBody()<<is_dctor_processed;
        if (is_dctor(method_name, decl) && is_dctor_processed == 0) {
            is_dctor_processed ++;
        } else {
            //  qDebug()<<"drop by isdefined:"<<method_name;
            // return;
        }
        // }
    // fix dup method generate
    if (decl->isOutOfLine()) {
        qDebug()<<"drop by outofline:"<<method_name;
        return;
    }
    if (!(decl->getAccess() == clang::AS_public
          || decl->getAccess() == clang::AS_protected))
        { qDebug()<<"drop by private:"<<method_name; return; } // omit  private method 
    if (is_oper(method_name)) { qDebug()<<"drop by operator:"<<method_name; return;}
    if (is_metar(method_name)) { has_meta_object = true; qDebug()<<"drop by meta:"<<method_name; return;}
    if (is_template_method(decl)) { qDebug()<<"drop by template:"<<method_name; return;}
    if (is_dptr(method_name)) { qDebug()<<"drop by dptr:"<<method_name; return;}

    return_type_str = trim_type_class(fret_type.getAsString().c_str());
    if (return_type_str.endsWith("iterator"))
        return_type_str = QString("%1::%2").arg(klass_name).arg(return_type_str);
    if (return_type_str == "_Bool") return_type_str = "bool";
    if (is_ctor(method_name)) new_method_name = QString("Ya%1").arg(klass_name);
    if (is_dtor(method_name)) new_method_name = QString("~Ya%1").arg(klass_name);

    // template: [public]: [static] [virtual] [explict] [type] name (
    tmp += QString("%1: %2 %3 %4 %5 %6 (")
        .arg(decl->getAccess() == clang::AS_public ? "public" : "protected")
        .arg(decl->isStatic() ? "static" : "")
        .arg(decl->isVirtual() ? "virtual" : "")
        .arg(is_ctor(method_name) && decl_m2ctor(decl)->isExplicit() ? "explicit" : "")
        .arg(is_ctor(method_name) || is_dtor(method_name) 
             ? "" : return_type_str)
        .arg(is_ctor(method_name) || is_dtor(method_name) ? new_method_name : method_name );


    // template: [type] klass::method [:pname(a1,a2)](
    ctmp += QString("%1%2 Ya%3::%4(")
        .arg(is_typedef_type(fret_type) && is_currclass_typedef(fret_type) ?
             QString("%1::").arg(klass_name) : "") // 认为是当前类的
        .arg(is_ctor(method_name) || is_dtor(method_name) 
             ? "" : (is_qflag_type(fret_type) ? get_qflag_name(fret_type) : return_type_str))
        .arg(klass_name)
        .arg(is_ctor(method_name) || is_dtor(method_name) ? new_method_name : method_name );

    // proto, 
    // template: [type] klass::method [:pname(a1,a2)](
    ptmp += QString("%1 Ya%2::%3(")
        .arg(is_ctor(method_name) || is_dtor(method_name) 
             ? "" : (is_qflag_type(fret_type) ? get_qflag_name(fret_type) : return_type_str))
        .arg(klass_name)
        .arg(is_ctor(method_name) || is_dtor(method_name) ? new_method_name : method_name );

    QVector<QString> auto_params;
    int param_size = decl->param_size();
    for (int i = 0; i < param_size; i ++) {
        const clang::ParmVarDecl *param = decl->getParamDecl(i);
        // param->dump();
        clang::SourceLocation loc = param->getLocStart();
        std::string pastr = loc.printToString(sm);
        // qDebug()<<"mydump:"<<pastr.c_str();
        clang::QualType param_type = param->getOriginalType();
        qDebug()<<"param type:"<<i<<param->hasDefaultArg()<<","<<param_type.getAsString().c_str()
                <<param->getNameAsString().c_str()
                <<"has defalut:"<<param->hasDefaultArg()
                <<"darg:"<<param->getDefaultArg()
                <<"type class name:"<<param_type->getTypeClassName();
        // param_type->dump(); 
        // fix private member problem, omit this arg
        if ((klass_name == "QObject" || klass_name=="QCoreApplication"
             || klass_name == "QTimer" || klass_name == "QThread")
            && QString(param_type->getTypeClassName()) == "Record") {
            drop_cond_count ++;
            continue;
        }

        // default arg code
        const clang::Expr *darg = param->getDefaultArg();
        QString default_arg_str = "";
        default_arg_str = def_arg_string(darg);

        // fix _Bool => bool
        QString arg_type_str = trim_type_class(param_type.getAsString().c_str());
        if (arg_type_str.contains("_Bool")) arg_type_str.replace("_Bool", "bool");

        // fix empty arg name
        QString empty_arg_name = param->getNameAsString().length() == 0
            ? QString("%1%2").arg(empty_arg_prefix).arg(++empty_arg_cnter)
            : QString(param->getNameAsString().c_str());
        auto_params.append(empty_arg_name);
        
        // method arglist code template: type name, type name,
        tmp += QString("%1 %2%3%4")
            .arg(arg_type_str)
            .arg(empty_arg_name)
            .arg(param->hasDefaultArg() ? QString(" = %1").arg(default_arg_str) : QString())
            .arg((i == param_size-1) ? "" : ", ");

        ctmp += QString("%1 %2%3")
            .arg(arg_type_str)
            .arg(empty_arg_name)
            .arg((i == param_size-1) ? "" : ", ");

        // protos
        ptmp += QString("%1 %2")
            .arg(arg_type_str)
            .arg((i == param_size-1) ? "" : ",");

    }

    if (is_ctor(method_name)) {
        // ctor init code template: type name, type name,
        ctmp += QString(") : %1(").arg(method_name);
        for (int i = 0; i < param_size; i ++) {
            const clang::ParmVarDecl *param = decl->getParamDecl(i);
            ctmp += QString("%1%2").arg(auto_params[i])
                .arg((i == param_size-1) ? "" : ", ");
        }
    }

    // template:  ) [const] [\n] {
    tmp += QString (") %1; \n")
        .arg(decl->isConst() ? "const" : "");

    ctmp += QString(") %1 %2{")
        .arg(decl->isConst() ? "const" : "")
        .arg((ctmp.length() > 80) ? QString("\n    ") : "");

    ptmp += QString (") %1")
        .arg(decl->isConst() ? "const" : "");

    if (is_ctor(method_name) || is_dtor(method_name)) {
    } else {
        // code template: [return] parent_class::method_name(;
        ctmp += QString("%1 %2::%3(").arg(fret_type->isVoidType() ? "/*void*/" : "return")
            .arg(klass_name)
            .arg(decl->getNameInfo().getAsString().c_str());
    
        for (int i = 0; i < param_size; i ++) {
            const clang::ParmVarDecl *param = decl->getParamDecl(i);
            clang::QualType param_type = param->getOriginalType();
            // qDebug()<<"param type:"<<i<<param->hasDefaultArg()<<","<<param_type.getAsString().c_str()
            //      <<param->getNameAsString().c_str();
            // fix private member problem, omit this arg
            if ((klass_name == "QObject" || klass_name=="QCoreApplication"
                 || klass_name == "QTimer" || klass_name == "QThread")
                && QString(param_type->getTypeClassName()) == "Record") {
                drop_cond_count ++;
                continue;
            }

            // code template: arg1, arg2, arg3...;
            ctmp += QString("%1%2").arg(auto_params[i])
                .arg((i == param_size-1) ? "" : ", ");
        }
        
        ctmp += QString("); ");
    }

    ctmp += QString(" }\n");

    // last fix
    if (drop_cond_count > 0) {
        qDebug()<<"drop by drop cond, private signal:"<<method_name;
        return;
    }

    gened_code += tmp;
    pctx.gened_cpp_code += ctmp;
    pctx.gened_proto_code += QString("{\"%1\",\"%2\"},\n").arg(ptmp)
        .arg(is_ctor(method_name) || is_dtor(method_name) 
             ? "" : (is_qflag_type(fret_type) ? get_qflag_name(fret_type) : return_type_str));
}

/*
  目标，解析出属于当前类的public enum，并用Q_ENUMS抛出可动态处理的方式。
  
 */
static void parser_enum(const clang::EnumDecl *ed)
{
    // this ctx very big
    const clang::DeclContext *dctx = ed->getDeclContext();
    const clang::Decl *de = clang::Decl::castFromDeclContext(dctx);
    qDebug()<<"dctx:"<<dctx<<de;
    // de->dump();

    const clang::DeclContext *pdctx = ed->getParent();
    const clang::Decl *de2 = clang::Decl::castFromDeclContext(pdctx);
    qDebug()<<"pdctx:"<<pdctx<<llvm::isa<clang::CXXRecordDecl>(de2);
    if (!llvm::isa<clang::CXXRecordDecl>(de2)) {
        return;
    }

    const clang::CXXRecordDecl *cxxrd = llvm::cast<clang::CXXRecordDecl>(de2);
    QString enum_of_class = QString(cxxrd->getName().data());
    if (enum_of_class != klass_name) {
        return;
    }

    qDebug()<<"found enum's class, hoho"<<enum_of_class;
    
    // next iterator enum_decls
    // it should be EnumConstantDecl type
    QString tmp;
    QString detmp;
    auto dit = ed->decls_begin();
    for (;dit != ed->decls_end(); dit++) {
        const clang::EnumConstantDecl *ecd = llvm::cast<clang::EnumConstantDecl>(*dit);
        ecd->dump();
        qDebug()<<"exname:"<<ecd->getName().data();
        // template: exname = class_name::exname,
        tmp += QString("y%1 = %2::%1,\n    ").arg(ecd->getName().data()).arg(klass_name);
    }

    // public: Q_ENUMS()
    // public: enum edname { enums } ;
    detmp = QString("Q_ENUMS(y%1);\n").arg(ed->getName().data());
    tmp = QString("public: enum y%1 {\n    %2 };\n")
        .arg(ed->getName().data()).arg(tmp);

    qDebug()<<"enum code here:"<<detmp<<tmp;

    pctx.qenums_code += detmp;
    // gened_code += tmp; // 暂时不把生成的enum放在生成的代码中，现在只需要符号表

    return;
    ///
    if (QString(ed->getName().data()) == "SpecialAddress") {
        const clang::CXXRecordDecl *cxxrd = llvm::cast<clang::CXXRecordDecl>(de2);
        qDebug()<<cxxrd;
        qDebug()<<cxxrd->getName().data();
        de2->dump();
    }
    
    // const clang::Decl *pd = clang::Decl::castFromDeclContext(pdctx);
    // qDebug()<<"is cxx record:"<<clang::isa<clang::CXXRecordDecl>(pdctx);
}

/*
  还需要考虑其基类，使用matcher不容易搞了，可能要考虑使用ASTConsumer了，这样能对AST执行更好的遍历。
 */
static void parser_decl_for_all(const clang::CXXMethodDecl *md)
{
    qDebug()<<"md:"<<md<<md->getDeclContext()->getDeclKindName();
    const clang::Decl *dd = clang::Decl::castFromDeclContext(md->getDeclContext());
    const clang::CXXRecordDecl *rd = llvm::cast<clang::CXXRecordDecl>(md->getDeclContext());
    qDebug()<<"rdname:"<<rd->getName().data()<<"base class:"<<rd->getNumBases();
    int method_count = 0;
    for (auto mit = rd->method_begin(); mit != rd->method_end(); mit++) {
        method_count ++;
    }
    qDebug()<<"method_count:"<<method_count;
    // 这个method只会包含在这个类中定义的方法，不包含基类中定义的方法。
    
}

static clang::CompilerInstance *pci = NULL;
static void get_class_method(QString module_name, QString class_name, QString file_path)
{
    clang::CompilerInstance ci;
    ci.createDiagnostics();

    ci.createFileManager();
    ci.createSourceManager(ci.getFileManager());

    pci = &ci;

    qDebug()<<"has source manager:"<<ci.hasSourceManager();

    using namespace clang;
    using namespace clang::ast_matchers;

    // OKed
    DeclarationMatcher MethodMatcher = 
        methodDecl(ofClass(hasName(class_name.toLatin1().data())))   // methodDecl(ofClass(hasName("QString")))
        .bind("matchMethod");

    class MethodPrinter : public MatchFinder::MatchCallback {
    public :
        virtual void run(const MatchFinder::MatchResult &Result) {
            auto nodes = Result.Nodes.getMap();
            foreach (auto node, nodes) {
                // qDebug()<<node.first.c_str()<<nodes.size();
            }
            const CXXMethodDecl *st = Result.Nodes.getNodeAs<CXXMethodDecl>("matchMethod");
            // qDebug()<<st << st->getNameInfo().getAsString().c_str();
            // st->dump();
            const clang::ASTContext &ast_ctx = st->getASTContext();
            // clang::SourceManager sm = ast_ctx->getSourceManager();
            parser_decl_for_one(st, ast_ctx, ast_ctx.getSourceManager());
            qDebug()<<pci->getSourceManager().getFilename(st->getLocStart()).data();
        }
    };

    DeclarationMatcher EnumMatcher =
        enumDecl()
        .bind("matchEnum");

    

    class EnumPrinter : public MatchFinder::MatchCallback {
    public:
        virtual void run(const MatchFinder::MatchResult &Result) {
            const EnumDecl *ed = Result.Nodes.getNodeAs<EnumDecl>("matchEnum");
            qDebug()<<"enum found:"<<ed<<"scoped:"<<ed->isScoped()
                    <<"completed:"<<ed->isComplete()
                // <<"promotype:"<<ed->getPromotionType()->getTypeClassName()
                // <<"inttype:"<<ed->getIntegerType()->getTypeClassName()
                //  <<"isfixed:"<<ed->isFixed()
                    <<"name?:"<<ed->getNameAsString().c_str()
                    <<"name1?:"<<ed->getName().data()
                    <<"\n"
                    <<"isc++:"<<ed->isCXXClassMember()
                    <<"declctx:"<<ed->getDeclContext()
                    <<"declkindname:"<<ed->getDeclContext()->getDeclKindName()
                    <<"decl name:";
            parser_enum(ed);
            // ed->decls_begin()->dump(); //遍历enumconst
            // ed->dump();
        }
    };

    DeclarationMatcher AllMethodMatcher = methodDecl().bind("allMethodMatch");
    class AllMethodPrinter : public MatchFinder::MatchCallback {
    public:
        virtual void run(const MatchFinder::MatchResult &Result) {
            const CXXMethodDecl *md = Result.Nodes.getNodeAs<CXXMethodDecl>("allMethodMatch");
            parser_decl_for_all(md);
        }
    };

    // 注，namespace名字全是小写的，类名首字母全是大写的
    const char *efile_path = "/usr/include/qt/QtCore/qstring.h";
    char hefile_path[100] = {0};
    strncpy(hefile_path, file_path.toLatin1().data(), file_path.length());
    efile_path = hefile_path;
    const char *argv[] = {
        "anoncxx", efile_path /*"/usr/include/qt/QtCore/qstring.h"*/, "--",
        // "-Xclang", "-ast-dump", "-fsyntax-only", 
        "-x", "c++", "-fPIC",
        "-I/usr/include/qt", 
        "-I/usr/lib/clang/3.5.0/include/",
    };
    int argc = 8;

    static llvm::cl::OptionCategory MyToolCategory("my-tool options");
    clang::tooling::CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);

    clang::tooling::ClangTool Tool(OptionsParser.getCompilations(),
                                   OptionsParser.getSourcePathList());


    MethodPrinter mPrinter;
    AllMethodPrinter amPrinter;
    EnumPrinter ePrinter;
    MatchFinder Finder;
    Finder.addMatcher(MethodMatcher, &mPrinter);
    Finder.addMatcher(EnumMatcher, &ePrinter);
    Finder.addMatcher(AllMethodMatcher, &amPrinter);

    bool bret = Tool.run(clang::tooling::newFrontendActionFactory<>(&Finder).get());
    // Tool.run(clang::tooling::newFrontendActionFactory<clang::SyntaxOnlyAction>().get());

    // return bret;
}

/*
  parser module_name class_name
  eg. ./qtparser QtCore QString /usr/include/qt/QtCore/qstring.h
  
  note:
  stderr is log message
  stdout is result code
 */
int main(int argc, char **argv)
{
    if (argc != 4) {
        qDebug()<<"need 4 arguments.";
        return -1;
    }

    ::module_name = QString(argv[1]);
    ::klass_name = QString(argv[2]); // "QString";
    ::file_path = QString(argv[3]); 
    
    get_class_method(module_name, klass_name, file_path);

    // template: class y##classname : [public QObject, ]public classname { Q_OBJECT; \n public slots: \n code_content };
    QString code_content = QString("#include <QtCore>\n"
                                   "#include <QtGui>\n"
                                   "#include <QtWidgets>\n"
                                   "#include <QtNetwork>\n"
                                   "class Ya%1 : %2 public %3"
                                   // "\n{Q_OBJECT;\n%4\npublic slots:\n"
                                   "\n{%4\npublic:\n"
                                   // "\npublic: Q_INVOKABLE y%4() : %5() {} \n"  // default ctor
                                   "%6 \n};")
        .arg(klass_name)
        .arg(has_meta_object ? "" : "") //"public QObject,")
        .arg(klass_name)
        .arg("") // pctx.qenums_code)
        // .arg(klass_name).arg(klass_name)
        .arg(gened_code);

    QString cpp_code_content = pctx.gened_cpp_code;

    fprintf(stdout, "%s\n", code_content.toLatin1().data());
    fprintf(stdout, "\n===========\n");
    fprintf(stdout, "%s\n", cpp_code_content.toLatin1().data());

    fprintf(stdout, "\n===========\n");
    fprintf(stdout, "%s\n", pctx.gened_proto_code.toLatin1().data());


    pctx.hfp.setFileName(QString("./ya%1.h").arg(klass_name).toLower());
    pctx.hfp.open(QIODevice::ReadWrite);
    pctx.hfp.resize(0);
    pctx.hfp.write(code_content.toLatin1());
    pctx.hfp.close();

    pctx.cfp.setFileName(QString("./ya%1.cpp").arg(klass_name).toLower());
    pctx.cfp.open(QIODevice::ReadWrite);
    pctx.cfp.resize(0);
    pctx.cfp.write(cpp_code_content.toLatin1());
    pctx.cfp.close();

    pctx.cfp.setFileName(QString("./ya%1.proto.cpp").arg(klass_name).toLower());
    pctx.cfp.open(QIODevice::ReadWrite);
    pctx.cfp.resize(0);
    pctx.cfp.write(pctx.gened_proto_code.toLatin1());
    pctx.cfp.close();


    return 0;
}
