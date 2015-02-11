#include "fix_clang_undef_eai.h"

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
}

#include "invokestorage.h"
InvokeStorage2 gis2;

llvm::Type *OperatorEngine::uniqTy(llvm::Module *mod, QString tyname)
{
    // 不再使用tymod中的类型，因为cmod中必须已经有这个类型了
    assert(mod != NULL);
    llvm::Type *ty1 = mod == NULL ? NULL
        : mod->getTypeByName(tyname.toLatin1().data());
    assert(ty1 != NULL && ty1 != 0);
    Q_ASSERT(ty1 != NULL);
    return ty1;
    
    // 现在生成的ll中有重复的类型定义，使用这个试试
    // 如果当前模块中已经有这个类型，则不在使用tymod中的类型
    // 这个应该有用，但目前看并不影响执行
    /*
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
    */
}

// 把用户传递过来的值转换成ll调用的值
std::vector<llvm::Value*>
OperatorEngine::ConvertToCallArgs(llvm::Module *module, llvm::IRBuilder<> &builder,
                                  QVector<QVariant> uargs, QVector<QVariant> dargs,
                                  llvm::Function *dstfun, bool has_this)
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

    // 处理EvalType类型参数
    QHash<QString, llvm::Value*> evals = this->darg_instcpy(module, builder);
    
    auto &func_params = dstfun->getArgumentList();
    qDebug()<<"param count:"<<func_params.size();
    auto fpit = func_params.begin();
    // skip sret param
    if (dstfun->hasStructRetAttr()) {
        fpit++;
    }
    // skip this param
    if (has_this) {
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
                ctypes.push_back(uniqTy(module, "class.QString")->getPointerTo());
                // 传这个值老是crash，是因为一直把is定义为局部变量了，从当前方法return后，这个is消失了。
                // 所以会有内存问题。
                gis2.sval[i] = QString(mrg_args.at(i).toString());
                // qDebug()<<"p1str addr:"<<(&gis.sval[i])<<(int64_t)(&gis.sval[i]);
                lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)&gis2.sval[i]);
                // 这不会和C++的VTable有关系吧。
                // 可能和返回值是值，而非引用或指针，没有相应的存储空间导致程序崩溃。
                lv = llvm::ConstantExpr::getIntToPtr(lc, uniqTy(module, "class.QString")->getPointerTo());
                // cargs.push_back(lv);
                // mfunc->addAttribute(i+1, llvm::Attribute::Dereferenceable); // i+1, for first this*
            }

            cargs.push_back(lv);
            break;

        case QMetaType::Int: case QMetaType::UInt:
        case QMetaType::Short: case QMetaType::UShort: {
            // TODO 对于Int*类型是不是要考虑databit确定是Int32还是Int64。
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
        case QMetaType::LongLong: case QMetaType::ULongLong:
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
            ctypes.push_back(uniqTy(module, "class.QChar")->getPointerTo());
            gis2.cval[i] = mrg_args.at(i).toChar();
            lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)&gis2.cval[i]);
            lv = llvm::ConstantExpr::getIntToPtr(lc, uniqTy(module, "class.QChar")->getPointerTo());
            cargs.push_back(lv);
            break;
        case QMetaType::VoidStar:
            ctypes.push_back(builder.getVoidTy()->getPointerTo());
            gis2.vval[i] = QVariant::fromValue(tmer).value<void*>(); // mrg_args.at(i).value<void*>();
            gis2.vval[i] = mrg_args.at(i).value<void*>(); // mrg_args.at(i).value<void*>();
            lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)gis2.vval[i]);
            // lv = llvm::ConstantExpr::getIntToPtr(lc, builder.getVoidTy()->getPointerTo());
            lv = llvm::ConstantExpr::getIntToPtr(lc, aty);
            if (gis2.vval[i] != NULL) {
                // lv = llvm::ConstantExpr::getIntToPtr(lc, aty);
            } else {
                // TODO 这个处理合适吗？
                // lv = builder.getInt32((int64_t)(0));
            }
            cargs.push_back(lv);
            break;
        case QMetaType::QStringList: {
            if (aty == builder.getInt8Ty()->getPointerTo()->getPointerTo()) {
                int cnter = 0;
                for (auto s: mrg_args.at(i).toStringList()) {
                    strcpy(gis2.csval[cnter++], s.toLatin1().data());
                }
                lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)gis2.csval);
                lv = llvm::ConstantExpr::getIntToPtr(lc, aty); // for byval param, 这个正确，但是void*则不正确
                cargs.push_back(lv);
            } else {
                gis2.slval[i] = mrg_args.at(i).toStringList();
                lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)&gis2.slval[i]);
                lv = llvm::ConstantExpr::getIntToPtr(lc, aty);
                cargs.push_back(lv);
            }
        }; break;
        default:
            if (v.userType() != EvalType::id) {
                qDebug()<<"not known type:"<<v<<v.type();
                break;
            }
                
            // TOOOOOOOOOOOOOOOOOODO
            EvalType r = v.value<EvalType>();
            QString argval = QString("toargx%1").arg(i); // %this为0占掉1个参数位置
                
            // 为什么64位和32位的传参方式不同呢？？？
            if (sizeof(void*) == 8) {
                if (r.vf_base_name.startsWith("_ZN6QFlagsIN2Qt")) {
                    std::vector<llvm::Value*> idxList = {builder.getInt32(0), builder.getInt32(0)};
                    llvm::Value* tlc = builder.CreateGEP(evals.value(argval), idxList);
                    lv = builder.CreateLoad(tlc);
                } else {
                    lv = builder.getInt32(0);
                }
                cargs.push_back(lv);
            } else { // OS x86
                qDebug()<<"eval type:"<<v<<r.ve<<r.vv<<evals.contains(argval)<<argval;
                cargs.push_back(evals.value(argval));
            }
            // et_code();
            break;
        }
    }

    // TODO move EvalType codegen alone
    auto et_code = []() -> void {
    };

    return cargs;
}

bool OperatorEngine::instcpy()
{
    return false;
}

// TODO 更好的拷贝方式，现在是写死的两个参数
QHash<QString, llvm::Value*>
OperatorEngine::darg_instcpy(llvm::Module *mod, llvm::IRBuilder<> &builder)
{
    // @_Z15__jit_main_tmplv
    llvm::Function *src_fun = mod->getFunction("_Z15__jit_main_tmplv");
    QHash<QString, llvm::Value*> vals;

    if (!src_fun) {
        qDebug()<<"maybe not need darg_instcpy:";
        return vals;
    }
    
    // 暂时这个函数只有一个BasicBlock
    auto &blk = src_fun->getEntryBlock();
    for (auto &inst: blk.getInstList()) {
        qDebug()<<"inst:"<<&inst<<inst.hasName()<<inst.getName().data();
        inst.dump();
        QString name = inst.getName().data();
        if (name.startsWith("toargx")) {
            auto new_inst = inst.clone();
            auto rt_inst = builder.Insert(new_inst, inst.getName());
            vals[name] = rt_inst;
        } else if (inst.getOpcode() == llvm::Instruction::Call
                   || inst.getOpcode() == llvm::Instruction::Invoke) {
            llvm::CallInst *pcall_inst = llvm::cast<llvm::CallInst>(&inst);
            std::vector<llvm::Value*> call_args;
            auto new_inst = inst.clone();
            qDebug()<<inst.getParent()<<new_inst->getParent()
                    <<pcall_inst->getNumArgOperands()
                    <<call_args.size();

            auto call_inst = llvm::cast<llvm::CallInst>(new_inst);
            for (int i = 0; i < pcall_inst->getNumArgOperands(); i ++) {
                auto lv = call_inst->getArgOperand(i);
                QString aname = lv->getName().data();
                llvm::Type *axt = lv->getType();
                if (llvm::isa<llvm::ConstantInt>(lv)) {
                    auto ilv = llvm::cast<llvm::ConstantInt>(lv);
                    auto nlv = llvm::ConstantInt::get(axt, ilv->getValue());
                    nlv->setName(lv->getName());
                    call_args.push_back(nlv);
                }
                else if (llvm::isa<llvm::ConstantStruct>(lv)) {
                    qDebug()<<"structtttttt.";
                } else if (axt->isPointerTy()) {
                    qDebug()<<"pointer ty."<<lv->getName().data()
                            <<vals.contains(aname);
                    call_args.push_back(vals.value(aname));
                }
                qDebug()<<i<<lv<<lv->getName().data()<<axt->getTypeID()
                        <<lv->getValueID()
                        <<llvm::Value::ConstantIntVal;
            }
            llvm::Function *callee_func = call_inst->getCalledFunction();
            QString cname = callee_func->getName().data();
            // cname.replace("C1", "C2"); // 这个也许不需要替换，直接使用C1也行。
            llvm::Function *new_callee_func = mod->getFunction(cname.toStdString());

            call_inst = builder.CreateCall(new_callee_func,
                                           llvm::ArrayRef<llvm::Value*>(call_args));
        }
    }

    return vals;
}

// TODO dereferenced return and dereferenced params
QString OperatorEngine::bind(llvm::Module *mod, QString symbol, QString klass,
                             QVector<QVariant> uargs, QVector<QVariant> dargs,
                             bool is_static, void *kthis)
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
    } else if (kthis == NULL) {
    } else {
        llvm::Type *thisTy = this->uniqTy(mod, QString("class.%1").arg(klass));
        qDebug()<<klass<<thisTy;
        assert(thisTy != NULL);
        // TODO 想办法使用真实的thisTy类型
        if (thisTy == NULL) {
            thisTy = builder.getVoidTy()->getPointerTo();
        }
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
        ConvertToCallArgs(mod, builder, uargs, dargs, dstfun, !is_static && kthis != NULL);
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
            auto dlo = this->getDataLayout(mod);
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
    // TODO byval call and reference arg
    for (auto &a: dstfun->getArgumentList()) {
        qDebug()<<a.getArgNo()<<a.hasByValAttr()<<a.getDereferenceableBytes();
        
        if (a.getDereferenceableBytes() > 0) {
            llvm::AttrBuilder ab(llvm::Attribute::getWithDereferenceableBytes(mod->getContext(), a.getDereferenceableBytes()));
            llvm::AttributeSet sets;
            sets = sets.addAttributes(mod->getContext(), a.getArgNo()+1,
                                      llvm::AttributeSet::get(mod->getContext(), a.getArgNo()+1, ab));
            cval->setAttributes(sets);
        }
        
        if (a.hasByValAttr()) {
            auto aty = a.getType();
            std::string ostr; llvm::raw_string_ostream ostm(ostr); aty->print(ostm);
            /*
            qDebug()<<"need byval attr:"<<(&a)<<dstfun->getName().data()
                    <<ostm.str().c_str()
                    <<a.getType()->isPointerTy()
                    <<a.getType()->isIntegerTy()
                    <<a.getArgNo();
            */
            if (a.getType()->isPointerTy()) {
                 // need +1, idx0为返回值属性,idx~0为函数属性。是否需要考虑sret？
                cval->addAttribute(a.getArgNo()+1, llvm::Attribute::ByVal);
            }
        }
    }
    // test for qflags byval
    if (false && symbol.indexOf("path") != -1 && klass == "QUrl") {
        cval->addAttribute(3, llvm::Attribute::ByVal);
        // qDebug()<<"QUrl::path called";
    }
    // for test
    if (false && symbol.indexOf("_ZN7QWidgetC2EPS_6QFlagsIN2Qt10WindowTypeEE") != -1) {
        cval->addAttribute(3, llvm::Attribute::ByVal);
    }

    // 针对有些需要返回record类对象的方法，却返回了i32，这时需要做一个后处理。see issue #2。
    auto elem_or_record_post_retval =
        [](llvm::IRBuilder<> &builder, llvm::Function *dstfun, llvm::Value *cval) {
        // int to object result
        // qDebug()<<builder.getInt32Ty()->getPrimitiveSizeInBits();
        void *ioret = calloc(1, builder.getInt32Ty()->getPrimitiveSizeInBits()/8);
        memset(ioret, 0, builder.getInt32Ty()->getPrimitiveSizeInBits()/8);
        llvm::Constant *lcv = builder.getInt64((int64_t)ioret);
        llvm::Value *lvv = llvm::ConstantExpr::getIntToPtr(lcv, builder.getVoidTy()->getPointerTo());
        // llvm::Value *unrefv = builder.CreateLoad(lvv);
        llvm::Value *stv = builder.CreateStore(cval, lvv);
        builder.CreateRet(lvv);
    };
    
    if (dstfun->hasStructRetAttr()) {
        builder.CreateRet(callee_arg_values.at(0));
    } else if (dstfun->doesNotReturn() || dstfun->getReturnType() == builder.getVoidTy()) {
        builder.CreateRetVoid();
    } else {
        // 还有一种方式，在ctrlengine中实现的，目前先不在这实现。
        // 不过ctrlengine和OperatorEngine两边差不多复杂，只是OperatorEngine还需要传递额外的数据才能处理。
        // 放在这里处理的好处是只需要改这一个地方，在ctrlengine中所有的Clvm::execute2调用处都需要处理。
        if (symbol == "_ZNK7QWidget10sizePolicyEv") {
            // elem_or_record_post_retval(builder, dstfun, cval);
        } else {

        }
        builder.CreateRet(cval);
    }

    qDebug()<<"operator bind done."<<lamsym;
    mod->dump();

    lamsym = QString(lamname);
    return lamsym;
}

void OperatorEngine::elem_or_record_post_return()
{
}

int OperatorEngine::getClassAllocSize(llvm::Module *mod, QString klass)
{
    // auto &dlo = mtmod->getDataLayout();
    auto dlo = this->getDataLayout(mod);
    auto kty = this->uniqTy(mod, QString("class.%1").arg(klass));

    return kty ? dlo->getTypeAllocSize(kty) : -1;
}

llvm::DataLayout *OperatorEngine::getDataLayout(llvm::Module *mod)
{
    // TODO dynamic datalayout
    static llvm::DataLayout *pdlo = NULL;
    if (pdlo == NULL) {
        // pdlo = new llvm::DataLayout("e-m:e-i64:64-f80:128-n8:16:32:64-S128");
        pdlo = new llvm::DataLayout(*mod->getDataLayout());
    }
    // qDebug()<<mod->getDataLayout()->getStringRepresentation().c_str();
    return pdlo;
}
