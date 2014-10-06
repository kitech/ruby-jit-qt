
#include "llvm/Support/SourceMgr.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/TypeBuilder.h"

#include "metalize/metas.h"
#include "clvm_operator.h"


bool irop_new(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder, 
            llvm::Module *module, QString klass)
{
    // test new operator
    llvm::Type *TQString = module->getTypeByName("class.YaQString");
    llvm::Type *TQklass = module->getTypeByName(QString("class.%1").arg(klass).toStdString());
    qDebug()<<"type:"<<TQString<<TQklass;
    TQString->dump();

    // new op
    std::vector<llvm::Type*> fargs = {TQString->getPointerTo()};
    llvm::ArrayRef<llvm::Type*> rfargs(fargs);
    llvm::FunctionType *funt = llvm::FunctionType::get(builder.getVoidTy(), rfargs, false);
    llvm::Constant *func = module->getOrInsertFunction("_ZN9YaQStringC1Ev", funt);

    // new func
    std::vector<llvm::Type*> new_fargs = {builder.getInt32Ty()};
    llvm::ArrayRef<llvm::Type*> new_rfargs(new_fargs);
    llvm::FunctionType *new_funt = llvm::FunctionType::get(builder.getInt8Ty()->getPointerTo(), new_rfargs, false);
    llvm::Constant *new_func = module->getOrInsertFunction("_Znwj",new_funt);

    llvm::Value *val = builder.CreateAlloca(TQString->getPointerTo());
    llvm::Value *mval = builder.CreateCall(new_func, builder.getInt32(1));
    llvm::Value *cval = builder.CreateBitCast(mval, TQString->getPointerTo());
    llvm::Value *val2 = builder.CreateStore(cval, val);

    llvm::LoadInst *rval = builder.CreateLoad(val);
    builder.CreateCall(func, rval);
    // builder.CreateRet(builder.getInt32(123));
    builder.CreateRet(rval);
    
    return true;
}

QString find_mangled_name(QString klass, QString method)
{
    static QVector<QString> symbols = {
        "_ZNK9YaQString6lengthEv",
    };

    QRegExp exp(QString("_Z.*Ya%1.*%2.*").arg(klass).arg(method));
    for (int i = 0; i < symbols.count(); i ++) {
        QString sym = symbols.at(i);

        if (exp.exactMatch(sym)) {
            // qDebug()<<exp.capturedTexts();
            return sym;
        }
    }
    return QString();
}

#include <cxxabi.h>
/*
  当前进程的symbol列表怎么动态获取？
  // nm -s ../libhandby.so |grep "YaQ"| awk '{print "\""$3"\","}' > ../jit_symbols.txt
 */
QString resolve_mangle_name(QString klass, QString method, QVector<QVariant> args)
{
    static QVector<QString> symbols2 = {
        #include "jit_symbols.txt"  // very good
    };
    qDebug()<<symbols2.count();

    QRegExp exp(QString("_Z(.*)%1Ya%2%3%4(.*)")
                .arg(klass.length() + 2).arg(klass)
                .arg(method.length()).arg(method));

    QVector<QString> results;
    // 1st round, 找到名字匹配的项
    for (int i = 0; i < symbols2.count(); i ++) {
        QString sym = symbols2.at(i);

        if (exp.exactMatch(sym)) {
            qDebug()<<exp.capturedTexts();
            // return sym;
            results.append(sym);
        }
    }

    auto demangle = [](QString mname) -> QString {
        size_t dmlen = 128*2;
        char *dmname = (char*)calloc(1, dmlen);
        int dmstatus = -1;
        __cxxabiv1::__cxa_demangle(mname.toLatin1().data(), dmname, &dmlen, &dmstatus);
        qDebug()<<dmlen<<dmstatus;
        if (dmstatus == 0) {
            return QString(dmname);
        }
        return QString();
    };

    QVector<QString> results2;
    // 2nd round, 找到参数个数匹配的，至少比给定的参数要多
    for (int i = 0; i < results.count(); i ++) {
        QString sym = results.at(i);
        QString dmname = demangle(sym);
        QStringList npart = dmname.mid(dmname.indexOf('(') + 1,
                                       dmname.lastIndexOf(')') - dmname.indexOf('(') - 1
                                       ).split(",");

        qDebug()<<npart;
        if (npart.count() >= args.count()) {
            results2.append(sym);
        }
    }


    auto presig = [](QVector<QVariant> args) -> QString {
        QString sig("void asig(");
        for (int i = 0; i < args.count(); i ++) {
            switch(args.at(i).type()) {
            case QMetaType::Int: sig += ("int,"); break;
            case QMetaType::Bool: sig += ("bool,"); break;
            case QMetaType::QString: sig += ("QString,"); break;
            default:
                break;
            }
        }
        sig += (")");
        return sig;
    };
    QString srcsig = presig(args);
    qDebug()<<"srcsig:"<<srcsig;

    QVector<QString> results3;
    // 3rd round, 找到参数类型匹配的
    for (int i = 0; i < results2.count(); i++) {
        QString sym = results.at(i);
        QString dmname = demangle(sym);
        QStringList npart = dmname.mid(dmname.indexOf('(') + 1,
                                       dmname.lastIndexOf(')') - dmname.indexOf('(') - 1
                                       ).split(",");
        qDebug()<<sym<<npart;
        
        QString aimslot = QString("void %1")
            .arg(QString(QMetaObject::normalizedSignature(dmname.split("::").at(1).toLatin1().data())));
        qDebug()<<"aimslot:"<<aimslot;
        bool sigmatch = QMetaObject::checkConnectArgs(srcsig.toLatin1().data(),
                                                      aimslot.toLatin1().data());
        bool sigmatch2 = QMetaObject::checkConnectArgs("void asig(int)",
                                                       "void aslot(long)");
        qDebug()<<"matched:"<<sigmatch<<sigmatch2;

        // 这个check得到的结果很不够。即使一个是int另一个是long，结果也不匹配。
        if (sigmatch) {
            results3.append(sym);
            // break; // 完全匹配了，则可以
        }

        QString norm_dmname = QMetaObject::normalizedSignature(dmname.split("::").at(1).toLatin1().data());
        QStringList norm_npart = norm_dmname.mid(norm_dmname.indexOf('(') + 1,
                                                 norm_dmname.lastIndexOf(')') - norm_dmname.indexOf('(') - 1
                                                 ).split(",");
        qDebug()<<"norm npart:"<<norm_npart;
        QBitArray matbit(args.count());
        for (int j = 0; j < args.count(); j++) {
            switch(args.at(j).type()) {
            case QMetaType::Int:
            case QMetaType::UInt:
            case QMetaType::Long:
            case QMetaType::ULong:
            case QMetaType::ULongLong:
            case QMetaType::LongLong:
            case QMetaType::Short:
            case QMetaType::UShort:
                if (norm_npart.at(j) == "int" || norm_npart.at(j) == "uint"
                    || norm_npart.at(j) == "long" || norm_npart.at(j) == "ulong"
                    || norm_npart.at(j) == "short" || norm_npart.at(j) == "ushort"
                    || norm_npart.at(j) == "qlonglong" || norm_npart.at(j) == "qulonglong") {
                    // matched j
                    matbit.setBit(j, true);
                }
                break;
            case QMetaType::Bool:
                if (norm_npart.at(j) == "bool") {
                    // matched
                    matbit.setBit(j, true);
                }
                break;
            case QMetaType::QString:
                if (norm_npart.at(j) == "QString" || norm_npart.at(j) == "char*") {
                    matbit.setBit(j, true);
                }
                break;
            case QMetaType::Double:
            case QMetaType::Float:
                if (norm_npart.at(j) == "double" || norm_npart.at(j) == "float") {
                    matbit.setBit(j, true);
                }
            default:
                break;
            }
        }
        qDebug()<<"matbit:"<<matbit;
        if (matbit.count(true) > 0) {
            results3.append(sym);
        }
    }

    qDebug()<<results.count()<<results2.count()<<results3.count()<<results3;
    if (results3.count() > 1) {
        qDebug()<<"found more than one match call, maybe really mismatch call";
    }

    if (results3.count() > 0) {
        return results3.at(0);
        return results3.at(qrand() % results3.count()); // not very good
    }

    return QString();
}

// 查找类方法函数的返回值
QString resolve_return_type(QString klass, QString method, QVector<QVariant> args, QString mangle_name)
{
    QHash<QString, QString> &protos = __rq_protos;
    qDebug()<<"protos count:"<<protos.count();

    auto demangle = [](QString mname) -> QString {
        size_t dmlen = 128*2;
        char *dmname = (char*)calloc(1, dmlen);
        int dmstatus = -1;
        __cxxabiv1::__cxa_demangle(mname.toLatin1().data(), dmname, &dmlen, &dmstatus);
        qDebug()<<dmlen<<dmstatus;
        if (dmstatus == 0) {
            return QString(dmname);
        }
        return QString();
    };

    QHash<QString, QString> results;
    QString dmname = demangle(mangle_name);
    dmname = QMetaObject::normalizedSignature(dmname.toLatin1().data());
    qDebug()<<"dmname:"<<dmname;
    for (auto it = protos.begin(); it != protos.end(); it ++) {
        QString k = it.key().trimmed();
        QString sig = k.right(k.length() - k.indexOf(" ")).trimmed();
        sig = QMetaObject::normalizedSignature(sig.toLatin1().data());
        if (sig.startsWith("&")) {
            sig = sig.right(sig.length()-1).trimmed();
        }
        if (dmname == sig) {
            qDebug()<<"found one:"<<k<<it.value();
            results[it.key()] = it.value();
        }
        if (sig.indexOf(QString("%1::%2").arg(klass).arg(method)) >= 0) {
            qDebug()<<"candicate??:"<<sig;
        }
    }

    qDebug()<<"found return type:"<<results.count();

    if (results.count() > 1) {
        qDebug()<<"mayby has some fault:"<<"multi return type";
    }

    if (results.count() > 0) {
        return results.begin().value();
    }

    return QString();
}

/*
Value* CastPtrToLlvmPtr(Type* type, const void* ptr) {
    Constant* const_int = ConstantInt::get(Type::getInt64Ty(context()), (int64_t)ptr);
    return ConstantExpr::getIntToPtr(const_int, type);
}
*/

bool irop_call(llvm::LLVMContext &ctx, llvm::IRBuilder<> &builder, 
               llvm::Module *module, void *kthis, QString klass, QString method)
{
    // emu param
    llvm::Value *lv;
    QVector<QVariant> callee_params;
    std::vector<llvm::Type*> caller_arg_types;
    std::vector<llvm::Value*> caller_arg_values;
    for (int i = 0; i < callee_params.count(); i ++) {
        QVariant v = callee_params.at(i);
        switch (v.type()) {
        case QMetaType::Int:
            lv = builder.getInt32(v.toInt());
            caller_arg_types.push_back(builder.getInt32Ty());
            break;
        case QMetaType::QString:
            // tolv(void *, klass)
            caller_arg_types.push_back(module->getTypeByName("class.QString"));
            break;
        case QMetaType::Bool:
            caller_arg_types.push_back(builder.getInt1Ty());
            caller_arg_values.push_back(builder.getInt1(v.toBool()));
            break;
        case QMetaType::Double:
            break;
        default:
            break;
        }
    }
    
    // find method function symbol
    QString symbol_name = QString("_Z%1%2%3%4").arg(klass.length()).arg(klass)
        .arg(method.length()).arg(method);
    klass = "QString";
    method = "length";
    symbol_name = find_mangled_name(klass, method);

    // declare the method func
    std::vector<llvm::Type*> mfargs = {builder.getInt32Ty()};
    llvm::ArrayRef<llvm::Type*> rmfargs(caller_arg_types); // (mfargs);
    llvm::FunctionType *mfunt = llvm::FunctionType::get(builder.getInt32Ty(), rmfargs, false);
    llvm::Constant *mfunc = module->getOrInsertFunction(symbol_name.toStdString(), mfunt);

    // load paramters
    

    // call method
    kthis = (void*) 123;
    llvm::Constant *pthisc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)kthis);
    llvm::Value *pthisv = llvm::ConstantExpr::getIntToPtr(pthisc, module->getTypeByName(QString("class.Ya%1").arg(klass).toStdString())->getPointerTo());
    caller_arg_values.insert(caller_arg_values.begin(), pthisv);
    llvm::ArrayRef<llvm::Value*> rmfvalues(caller_arg_values);
    llvm::CallInst *cval = builder.CreateCall(mfunc, rmfvalues);
    builder.CreateRet(cval);

    // return

    return true;
}


///////////////////////////////
////
///////////////////////////////
IROperator::IROperator()
    : ctx(llvm::getGlobalContext()),
      builder(ctx)
{
    // ctx = llvm::getGlobalContext();
    this->dmod = new llvm::Module("jittop", ctx);
    init();
}

IROperator::~IROperator()
{

}

bool IROperator::init()
{
    QFile fp("./jit_types.ll");
    fp.open(QIODevice::ReadOnly);
    QByteArray llcode = fp.readAll();
    fp.close();

    char *llcode_str = strdup(llcode.data());
    llvm::SMDiagnostic diag;
    tmod = llvm::ParseAssemblyString(llcode_str, NULL, diag, ctx);

    free(llcode_str);

    return true;
}


bool IROperator::knew(QString klass)
{
    llvm::Module *module = this->dmod;

    // test new operator
    llvm::Type *TQString = module->getTypeByName("class.YaQString");
    llvm::Type *TQklass = module->getTypeByName(QString("class.%1").arg(klass).toStdString());
    qDebug()<<"type:"<<TQString<<TQklass;
    TQString->dump();

    // new op
    std::vector<llvm::Type*> fargs = {TQString->getPointerTo()};
    llvm::ArrayRef<llvm::Type*> rfargs(fargs);
    llvm::FunctionType *funt = llvm::FunctionType::get(builder.getVoidTy(), rfargs, false);
    llvm::Constant *func = module->getOrInsertFunction("_ZN9YaQStringC1Ev", funt);

    // new func
    std::vector<llvm::Type*> new_fargs = {builder.getInt32Ty()};
    llvm::ArrayRef<llvm::Type*> new_rfargs(new_fargs);
    llvm::FunctionType *new_funt = llvm::FunctionType::get(builder.getInt8Ty()->getPointerTo(), new_rfargs, false);
    llvm::Constant *new_func = module->getOrInsertFunction("_Znwj",new_funt);

    llvm::Value *val = builder.CreateAlloca(TQString->getPointerTo());
    llvm::Value *mval = builder.CreateCall(new_func, builder.getInt32(1));
    llvm::Value *cval = builder.CreateBitCast(mval, TQString->getPointerTo());
    llvm::Value *val2 = builder.CreateStore(cval, val);

    llvm::LoadInst *rval = builder.CreateLoad(val);
    builder.CreateCall(func, rval);
    // builder.CreateRet(builder.getInt32(123));
    builder.CreateRet(rval);

    return true;
}


bool IROperator::kdelete(void *kthis, QString klass)
{
    return true;
}

typedef struct {
    int iretval;
    bool bretval;
    QString sretval;

    // 最大支持10个参数，参数的临时值放在这
    int ival[10];
    bool bval[10];
    QString sval[10];
} InvokeStorage;

InvokeStorage is;

bool IROperator::call(void *kthis, QString klass, QString method, QVector<QVariant> args,
                      QString param_symbol_name)
{
    llvm::Module *module = this->dmod;

    // test
    // QVector<QVariant> _args = {QString("abc")};
    // resolve_mangle_name(klass, "append", _args);
    QString mangle_name = "_ZN9YaQString6appendERK7QString";
    this->resolve_return_type(klass, "append", args, mangle_name);

    // emu param
    llvm::Value *lv;
    llvm::Constant *lc;
    QVector<QVariant> callee_params = args;
    std::vector<llvm::Type*> caller_arg_types;
    std::vector<llvm::Value*> caller_arg_values;
    QBitArray derefbit(callee_params.count());
    for (int i = 0; i < callee_params.count(); i ++) {
        QVariant v = callee_params.at(i);
        switch (v.type()) {
        case QMetaType::Int:
            lv = builder.getInt32(v.toInt());
            caller_arg_types.push_back(builder.getInt32Ty());
            break;
        case QMetaType::QString:
            // tolv(void *, klass)
            caller_arg_types.push_back(module->getTypeByName("class.QString")->getPointerTo());
            // 传这个值老是crash，是因为一直把is定义为局部变量了，从当前方法return后，这个is消失了。
            // 所以会有内存问题。
            is.sval[i] = callee_params.at(i).toString();
            lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)&is.sval[i]);
            lv = llvm::ConstantExpr::getIntToPtr(lc, module->getTypeByName("class.QString")->getPointerTo());
            caller_arg_values.push_back(lv);
            // mfunc->addAttribute(i+1, llvm::Attribute::Dereferenceable); // i+1, for first this*
            derefbit.setBit(i, true);
            break;
        case QMetaType::Bool:
            caller_arg_types.push_back(builder.getInt1Ty());
            caller_arg_values.push_back(builder.getInt1(v.toBool()));
            break;
        default:
            break;
        }
    }
    
    // find method function symbol
    QString symbol_name = QString("_Z%1%2%3%4").arg(klass.length()).arg(klass)
        .arg(method.length()).arg(method);
    klass = "QString";
    // method = "length";
    symbol_name = find_mangled_name(klass, method);
    symbol_name = resolve_mangle_name(klass, method, args);
    param_symbol_name = symbol_name;

    // declare the method func
    std::vector<llvm::Type*> mfargs = {builder.getInt32Ty()};
    caller_arg_types.insert(caller_arg_types.begin(),
                            module->getTypeByName(QString("class.Ya%1").arg(klass).toStdString())
                            ->getPointerTo());
    llvm::ArrayRef<llvm::Type*> rmfargs(caller_arg_types); // (mfargs);
    llvm::Type* frettype =  module->getTypeByName(QString("class.%1").arg(klass).toStdString())
        ->getPointerTo();
    // llvm::FunctionType *mfunt = llvm::FunctionType::get(builder.getInt32Ty(), rmfargs, false);
    llvm::FunctionType *mfunt = llvm::FunctionType::get(frettype, rmfargs, false);
    llvm::Constant *mfunc = module->getOrInsertFunction(symbol_name.toStdString(), mfunt);
    llvm::Function *real_mfunc = module->getFunction(symbol_name.toStdString());

    // load paramters
    // demangle symbol_name,按照callee的参数类型，但按照caller参数个数填入参数值。
    qDebug()<<derefbit;
    for (int i = 0; i < derefbit.count(); i ++) {
        if (derefbit.testBit(i)) {
            llvm::AttrBuilder ab(llvm::Attribute::getWithDereferenceableBytes(ctx, sizeof(void*)));
            real_mfunc->addAttributes(i+1, llvm::AttributeSet::get(ctx, 2, ab));
        }
    }

    // call method
    // kthis = (void*) 123;
    llvm::Constant *pthisc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)kthis);
    llvm::Value *pthisv = llvm::ConstantExpr::getIntToPtr(pthisc, module->getTypeByName(QString("class.Ya%1").arg(klass).toStdString())->getPointerTo());
    caller_arg_values.insert(caller_arg_values.begin(), pthisv);
    qDebug()<<"arguments count:"<<caller_arg_values.size();

    llvm::ArrayRef<llvm::Value*> rmfvalues(caller_arg_values);
    llvm::CallInst *cval = builder.CreateCall(mfunc, rmfvalues);

    // cval to int??? not need

    builder.CreateRet(cval);
    // builder.CreateRetVoid();

    return true;
}

// 查找类方法函数的返回值
QString IROperator::resolve_return_type(QString klass, QString method,
                                        QVector<QVariant> args, QString mangle_name)
{
    return ::resolve_return_type(klass, method, args, mangle_name);
}
