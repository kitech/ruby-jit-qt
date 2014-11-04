
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>

#include "operatorengine.h"

OperatorEngine::OperatorEngine()
{
    this->init();
}

void OperatorEngine::init()
{
    auto load_jit_types_module = []() -> llvm::Module* {
        llvm::LLVMContext *ctx = new llvm::LLVMContext();
        llvm::Module *module = new llvm::Module("jit_types_in_oe", *ctx);

        // load module
        QFile fp("./metalize/jit_types.ll");
        fp.open(QIODevice::ReadOnly);
        QByteArray asm_data = fp.readAll();
        fp.close();

        char *asm_str = strdup(asm_data.data());
        qDebug()<<"llstrlen:"<<strlen(asm_str);

        llvm::SMDiagnostic smdiag;
        llvm::Module *rmod = NULL;

        rmod = llvm::ParseAssemblyString(asm_str, NULL, smdiag, *ctx);
        qDebug()<<"rmod="<<rmod;
        free(asm_str);

        return rmod;
    };
    this->mtmod = load_jit_types_module();
}

#include "invokestorage.h"
InvokeStorage2 gis2;

llvm::Type *OperatorEngine::uniqTy(llvm::Module *mod, QString tyname)
{
    // 现在生成的ll中有重复的类型定义，使用这个试试
    // 如果当前模块中已经有这个类型，则不在使用tymod中的类型
    // 这个应该有用，但目前看并不影响执行
    auto getUniqTy = [](llvm::Module *cmod, llvm::Module *tymod, QString tyname) -> llvm::Type* {
        llvm::Type *ty1 = cmod == NULL ? NULL
            : cmod->getTypeByName(tyname.toLatin1().data());
        llvm::Type *ty2 = tymod->getTypeByName(tyname.toLatin1().data());

        if (ty1 == ty2) {
            qDebug()<<"nice, two type equal";
        }
        if (ty1 != NULL && ty1 != ty2) {
            qDebug()<<"two type not equal!!!";
        }
        return ty1 != NULL ? ty2 : ty2;
    };
    return getUniqTy(mod, mtmod, tyname);
}

// 把用户传递过来的值转换成ll调用的值
std::vector<llvm::Value*>
OperatorEngine::ConvertToCallArgs(llvm::Module *module, llvm::IRBuilder<> &builder,
                                  QVector<QVariant> uargs, QVector<QVariant> dargs,
                                  llvm::Module *tymod, llvm::Function *dstfun, bool is_static)
{
    std::vector<llvm::Value*> cargs;
    std::vector<llvm::Type*> ctypes;

    QVector<QVariant> mrg_args = uargs;
    mrg_args.resize(dargs.count());
    for (int i = 0; i < dargs.count(); i ++) {
        if (!mrg_args.at(i).isValid() && dargs.at(i).isValid()) {
            mrg_args[i] = dargs.at(i);
        }
    }

    auto &func_params = dstfun->getArgumentList();
    qDebug()<<"param count:"<<func_params.size();
    auto fpit = func_params.begin();
    // skip sret param
    if (dstfun->hasStructRetAttr()) {
        fpit++;
    }
    // skip this param
    if (!is_static) {
        fpit++;
    }

    // emu param
    llvm::Value *lv;
    llvm::Constant *lc;
    llvm::Type *aty;
    QString sty;
    QTimer *tmer = new QTimer();
    void *vtmer = tmer;
    for (int i = 0; i < mrg_args.count(); i ++, fpit++) {
        QVariant v = mrg_args.at(i);
        llvm::Argument &farg = *fpit;
        aty = (*fpit).getType();
        std::string ostr; llvm::raw_string_ostream ostm(ostr); aty->print(ostm);
        sty = QString(ostm.str().c_str());
        qDebug()<<"param real type:"<<sty<<aty<<builder.getInt8Ty()->getPointerTo()<<v;

        switch ((int)v.type()) {
        case QMetaType::QString:
            // if (sty == "i8*") {
            if (aty == builder.getInt8Ty()->getPointerTo()) {
                ctypes.push_back(aty);
                strcpy(gis2.csval[i], mrg_args.at(i).toString().toStdString().c_str());
                lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)gis2.csval[i]);
                lv = llvm::ConstantExpr::getIntToPtr(lc, aty);
                // cargs.push_back(lv);
                qDebug()<<(void*)gis2.csval[i]<<(int64_t)gis2.csval[i];
            } else {
                qDebug()<<"using QString param type";
                // ctypes.push_back(tymod->getTypeByName("class.QString")->getPointerTo());
                ctypes.push_back(uniqTy(module, "class.QString")->getPointerTo());
                // 传这个值老是crash，是因为一直把is定义为局部变量了，从当前方法return后，这个is消失了。
                // 所以会有内存问题。
                gis2.sval[i] = QString(mrg_args.at(i).toString());
                // qDebug()<<"p1str addr:"<<(&gis.sval[i])<<(int64_t)(&gis.sval[i]);
                lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)&gis2.sval[i]);
                // 这不会和C++的VTable有关系吧。
                // 可能和返回值是值，而非引用或指针，没有相应的存储空间导致程序崩溃。
                // lv = llvm::ConstantExpr::getIntToPtr(lc, tymod->getTypeByName("class.QString")->getPointerTo());
                lv = llvm::ConstantExpr::getIntToPtr(lc, uniqTy(module, "class.QString")->getPointerTo());
                // cargs.push_back(lv);
                // mfunc->addAttribute(i+1, llvm::Attribute::Dereferenceable); // i+1, for first this*
            }

            cargs.push_back(lv);
            break;

        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::Short:
        case QMetaType::UShort: {
            if (farg.getDereferenceableBytes() == 0) {
                ctypes.push_back(builder.getInt32Ty());
                lv = builder.getInt32(v.toInt());
            } else {
                ctypes.push_back(aty);
                gis2.ival[i] = v.toInt();
                lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)&gis2.ival[i]);
                lv = llvm::ConstantExpr::getIntToPtr(lc, aty);
            }
            cargs.push_back(lv);
        }; break;
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
            if (farg.getDereferenceableBytes() == 0) {
                ctypes.push_back(builder.getInt64Ty());
                lv = builder.getInt64(v.toLongLong());
            } else {
                ctypes.push_back(aty);
                gis2.lval[i] = v.toLongLong();
                lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)&gis2.lval[i]);
                lv = llvm::ConstantExpr::getIntToPtr(lc, aty);
            }
            cargs.push_back(lv);
            break;
        case QMetaType::Bool:
            ctypes.push_back(builder.getInt1Ty());
            cargs.push_back(builder.getInt1(v.toBool()));
            break;
        case QMetaType::QChar:
            // ctypes.push_back(tymod->getTypeByName("class.QChar")->getPointerTo());
            ctypes.push_back(uniqTy(module, "class.QChar")->getPointerTo());
            gis2.cval[i] = mrg_args.at(i).toChar();
            lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)&gis2.cval[i]);
            // lv = llvm::ConstantExpr::getIntToPtr(lc, tymod->getTypeByName("class.QChar")->getPointerTo());
            lv = llvm::ConstantExpr::getIntToPtr(lc, uniqTy(module, "class.QChar")->getPointerTo());
            cargs.push_back(lv);
            break;
        case QMetaType::VoidStar:
            ctypes.push_back(builder.getVoidTy()->getPointerTo());
            gis2.vval[i] = QVariant::fromValue(tmer).value<void*>(); // mrg_args.at(i).value<void*>();
            gis2.vval[i] = mrg_args.at(i).value<void*>(); // mrg_args.at(i).value<void*>();
            lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)gis2.vval[i]);
            // lv = llvm::ConstantExpr::getIntToPtr(lc, builder.getVoidTy()->getPointerTo());
            if (gis2.vval[i] == NULL) {
            lv = llvm::ConstantExpr::getIntToPtr(lc, builder.getVoidTy()->getPointerTo()); // for byval param, 这个正确，但是void*则不正确
            } else {
                lv = builder.getInt32((int64_t)(0));
            }
            cargs.push_back(lv);
            break;
        case QMetaType::QStringList: {
            int cnter = 0;
            for (auto s: mrg_args.at(i).toStringList()) {
                strcpy(gis2.csval[cnter++], s.toLatin1().data());
            }
            lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)gis2.csval);
            lv = llvm::ConstantExpr::getIntToPtr(lc, aty); // for byval param, 这个正确，但是void*则不正确
            cargs.push_back(lv);
        }; break;
        default:
            qDebug()<<"not known type:"<<v<<v.type();
            break;
        }
    }
    

    return cargs;
}

// TODO dereferenced return and dereferenced params
QString OperatorEngine::bind(llvm::Module *mod, QString symbol, void *kthis, QString klass,
                             QVector<QVariant> uargs, QVector<QVariant> dargs,
                             bool is_static)
{
    QString lamsym;

    llvm::LLVMContext &vmctx = mod->getContext();
    llvm::IRBuilder<> builder(vmctx);
    llvm::Function *dstfun = mod->getFunction(symbol.toLatin1().data());
    llvm::Function *lamfun = NULL;
    const char *lamname = "jit_main"; // TODO, rt mangle
    
    auto c_lamfun = mod->getOrInsertFunction(lamname, builder.getVoidTy()->getPointerTo(), NULL);
    lamfun = (decltype(lamfun))(c_lamfun); // cast it
    builder.SetInsertPoint(llvm::BasicBlock::Create(mod->getContext(), "eee", lamfun));

    std::vector<llvm::Value*> callee_arg_values;
    if (is_static) {
    } else {
        llvm::Type *thisTy = this->uniqTy(mod, QString("class.%1").arg(klass));
        // 在32位系统上crash，可能是因为这里直接使用了64位整数。(已验证，果然是这个问题）
        // 应该是在23位系统上不能直接使用int64_t表示指针，而64位系统可以。
        // 一般显示为llvm::TargetLowering::LowerCallTo中crash
        // callee_arg_values.push_back(llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)kthis));
        llvm::Constant *lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)kthis);
        llvm::Constant *lv = llvm::ConstantExpr::getIntToPtr(lc, thisTy->getPointerTo());
        callee_arg_values.push_back(lv);
    }

    // Add uarg
    std::vector<llvm::Value*> more_values = 
        ConvertToCallArgs(mod, builder, uargs, dargs, mtmod, dstfun, is_static);
    // std::copy(more_values.begin(), more_values.end(), callee_arg_values.end());// why???
    for (auto v: more_values) callee_arg_values.push_back(v);


    // deal sret
    qDebug()<<"sret:"<<dstfun->hasStructRetAttr()<<"noret:"<<dstfun->doesNotReturn();
    if (dstfun->hasStructRetAttr()) {
        // qDebug()<<"arg size:"<<dstfun->arg_size();
        auto &sret_arg = *dstfun->arg_begin();
        llvm::Type *sretype = dstfun->getReturnType(); // 如果是sret，则获取不到returntype了
        sretype = sret_arg.getType();
        std::string ostr; llvm::raw_string_ostream ostm(ostr);
        sretype->print(ostm);
        qDebug()<<"sret type:"<<ostm.str().c_str();

        if (ostm.str() == "%class.QString*") {
            gis2.sbyvalret = (decltype(gis2.sbyvalret))calloc(1, sizeof(decltype(*gis2.sbyvalret)));
            llvm::Constant *rlr1 = builder.getInt64((int64_t)gis2.sbyvalret);
            llvm::Value *rlr2 = llvm::ConstantExpr::getIntToPtr(rlr1, sretype);
            llvm::Value *rlr3 = builder.CreateAlloca(sretype, builder.getInt32(1), "oretaddr");
            llvm::Value *rlr4 = builder.CreateStore(rlr2, rlr3);
            llvm::Value *rlr5 = builder.CreateLoad(rlr3, "sret2");
            callee_arg_values.insert(callee_arg_values.begin(), rlr5);    
        } else {
            auto dlo = mtmod->getDataLayout();
            gis2.vbyvalret = calloc(1, dlo->getTypeAllocSize(sretype));
            llvm::Constant *rlr1 = builder.getInt64((int64_t)gis2.vbyvalret);
            llvm::Value *rlr2 = llvm::ConstantExpr::getIntToPtr(rlr1, sretype);
            llvm::Value *rlr3 = builder.CreateAlloca(sretype, builder.getInt32(1), "oretaddr");
            llvm::Value *rlr4 = builder.CreateStore(rlr2, rlr3);
            llvm::Value *rlr5 = builder.CreateLoad(rlr3, "sret2");
            callee_arg_values.insert(callee_arg_values.begin(), rlr5);    
        }
    }

    llvm::ArrayRef<llvm::Value*> callee_arg_values_ref(callee_arg_values);
    llvm::CallInst *cval = builder.CreateCall(dstfun, callee_arg_values_ref);

    // dereferenced test, QString::append
    // TODO 需要更通用一些，目前只针对特定的函数
    if (false) {
        llvm::AttrBuilder ab(llvm::Attribute::getWithDereferenceableBytes(mod->getContext(), 1*sizeof(void*)));
        llvm::AttributeSet sets;
        sets = sets.addAttributes(mod->getContext(), 2, llvm::AttributeSet::get(mod->getContext(), 2, ab));
        sets = sets.addAttributes(mod->getContext(), 0, llvm::AttributeSet::get(mod->getContext(), 0, ab));
        cval->setAttributes(sets);
    }
    // TODO byval call
    // test for qflags byval
    if (false && symbol.indexOf("path") != -1 && klass == "QUrl") {
        cval->addAttribute(3, llvm::Attribute::ByVal);
        // qDebug()<<"QUrl::path called";
    }
    
    if (dstfun->hasStructRetAttr()) {
        builder.CreateRet(callee_arg_values.at(0));
    } else if (dstfun->doesNotReturn() || dstfun->getReturnType() == builder.getVoidTy()) {
        builder.CreateRetVoid();
    } else {
        builder.CreateRet(cval);
    }

    mod->dump();

    lamsym = QString(lamname);
    return lamsym;
}

int OperatorEngine::getClassAllocSize(QString klass)
{
    auto dlo = mtmod->getDataLayout();
    // auto kty = mtmod->getTypeByName(QString("class.%1").arg(klass).toStdString());
    auto kty = this->uniqTy(NULL, QString("class.%1").arg(klass));

    if (0) {
        qDebug()<<dlo<<dlo->getTypeAllocSize(mtmod->getTypeByName("class.QString"));
        qDebug()<<dlo<<dlo->getTypeAllocSize(mtmod->getTypeByName("class.QByteArray"));
        qDebug()<<dlo<<dlo->getTypeAllocSize(mtmod->getTypeByName("class.QTimer.base"));
        qDebug()<<dlo<<dlo->getTypeAllocSize(mtmod->getTypeByName("class.QThread"));
    }

    return kty ? dlo->getTypeAllocSize(kty) : -1;
}

