
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/Support/SourceMgr.h>

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
InvokeStorage gis2;

// 把用户传递过来的值转换成ll调用的值
std::vector<llvm::Value*>
ConvertToCallArgs(llvm::Module *module, llvm::IRBuilder<> &builder,
                  QVector<QVariant> uargs, QVector<QVariant> dargs)
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
   
    // emu param
    llvm::Value *lv;
    llvm::Constant *lc;
    for (int i = 0; i < mrg_args.count(); i ++) {
        QVariant v = mrg_args.at(i);
        switch ((int)v.type()) {
        case QMetaType::QString:
            ctypes.push_back(module->getTypeByName("class.QString")->getPointerTo());
            // 传这个值老是crash，是因为一直把is定义为局部变量了，从当前方法return后，这个is消失了。
            // 所以会有内存问题。
            gis2.sval[i] = QString(mrg_args.at(i).toString());
            // qDebug()<<"p1str addr:"<<(&gis.sval[i])<<(int64_t)(&gis.sval[i]);
            lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)&gis2.sval[i]);
            // 这不会和C++的VTable有关系吧。
            // 可能和返回值是值，而非引用或指针，没有相应的存储空间导致程序崩溃。
            lv = llvm::ConstantExpr::getIntToPtr(lc, module->getTypeByName("class.QString")->getPointerTo());
            cargs.push_back(lv);
            // mfunc->addAttribute(i+1, llvm::Attribute::Dereferenceable); // i+1, for first this*
            break;
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::Short:
        case QMetaType::UShort:
            ctypes.push_back(builder.getInt32Ty());
            lv = builder.getInt32(v.toInt());
            cargs.push_back(lv);
            break;
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
            ctypes.push_back(builder.getInt64Ty());
            lv = builder.getInt64(v.toLongLong());
            cargs.push_back(lv);
            break;
        case QMetaType::Bool:
            ctypes.push_back(builder.getInt1Ty());
            cargs.push_back(builder.getInt1(v.toBool()));
            break;
        case QMetaType::QChar:
            ctypes.push_back(module->getTypeByName("class.QChar")->getPointerTo());
            gis2.cval[i] = mrg_args.at(i).toChar();
            lc = llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)&gis2.cval[i]);
            lv = llvm::ConstantExpr::getIntToPtr(lc, module->getTypeByName("class.QChar")->getPointerTo());
            cargs.push_back(lv);
            break;
        default:
            qDebug()<<"not known type:"<<v<<v.type();
            break;
        }
    }
    

    return cargs;
}

QString OperatorEngine::bind(llvm::Module *mod, QString symbol, void *kthis, QVector<QVariant> uargs
                             , QVector<QVariant> dargs)
{
    QString lamsym;

    llvm::LLVMContext &vmctx = mod->getContext();
    llvm::IRBuilder<> builder(vmctx);
    llvm::Function *dstfun = mod->getFunction(symbol.toLatin1().data());
    llvm::Function *lamfun = NULL;
    const char *lamname = "jit_main"; // TODO, rt mangle
    
    auto c_lamfun = mod->getOrInsertFunction(lamname, builder.getVoidTy()->getPointerTo(), NULL);
    lamfun = (decltype(lamfun))(c_lamfun); // cast it

    std::vector<llvm::Value*> callee_arg_values;
    QString klass = "QString";
    llvm::Type *thisTy = this->mtmod->getTypeByName(QString("class.%1").arg(klass).toStdString());
    callee_arg_values.push_back(llvm::ConstantInt::get(builder.getInt64Ty(), (int64_t)kthis));

    // add uarg
    std::vector<llvm::Value*> more_values = ConvertToCallArgs(mod, builder, uargs, dargs);
    std::copy(more_values.begin(), more_values.end(), callee_arg_values.begin()+1);

    // deal sret
    if (dstfun->hasStructRetAttr()) {
        llvm::Type *sretype = dstfun->getReturnType();
        if (mtmod->getTypeByName("class.QString") == sretype) {
            gis2.sbyvalret = (decltype(gis2.sbyvalret))calloc(1, sizeof(decltype(*gis2.sbyvalret)));
            llvm::Constant *rlr1 = builder.getInt64((int64_t)gis2.sbyvalret);
            llvm::Value *rlr2 = llvm::ConstantExpr::getIntToPtr(rlr1, sretype->getPointerTo());
            llvm::Value *rlr3 = builder.CreateAlloca(sretype->getPointerTo(), builder.getInt32(1), "oretaddr");
            llvm::Value *rlr4 = builder.CreateStore(rlr2, rlr3);
            llvm::Value *rlr5 = builder.CreateLoad(rlr3, "sret2");
            callee_arg_values.insert(callee_arg_values.begin(), rlr5);    
        }
    }

    builder.SetInsertPoint(llvm::BasicBlock::Create(mod->getContext(), "eee", lamfun));
    llvm::ArrayRef<llvm::Value*> callee_arg_values_ref(callee_arg_values);
    llvm::CallInst *cval = builder.CreateCall(dstfun, callee_arg_values_ref);

    qDebug()<<"sret:"<<dstfun->hasStructRetAttr()<<"noret:"<<dstfun->doesNotReturn();
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
    auto kty = mtmod->getTypeByName(QString("class.%1").arg(klass).toStdString());

    if (0) {
        qDebug()<<dlo<<dlo->getTypeAllocSize(mtmod->getTypeByName("class.QString"));
        qDebug()<<dlo<<dlo->getTypeAllocSize(mtmod->getTypeByName("class.QByteArray"));
        qDebug()<<dlo<<dlo->getTypeAllocSize(mtmod->getTypeByName("class.QTimer.base"));
        qDebug()<<dlo<<dlo->getTypeAllocSize(mtmod->getTypeByName("class.QThread"));
    }

    return kty ? dlo->getTypeAllocSize(kty) : -1;
}

