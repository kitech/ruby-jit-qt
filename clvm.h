#ifndef CLVM_H
#define CLVM_H

#include "fix_clang_undef_eai.h"

#include <QThread>
#include <llvm/ExecutionEngine/GenericValue.h>

namespace llvm {
    struct GenericValue;
    class ExecutionEngine;
    class EngineBuilder;
    class Module;
};

namespace clang {
    class CompilerInvocation;
    class CompilerInstance;

    namespace driver {
        class Driver;
    };
};

/////
class ClvmJitListener;

/////
llvm::GenericValue vm_execute(QString code, QVector<llvm::GenericValue> &envp);

void *jit_vm_new(QString klass, QVector<QVariant> args);
QVariant jit_vm_call(void *kthis, QString klass, QString method, QVector<QVariant> args);

class Clvm : public QThread
{
    Q_OBJECT;
public:
    Clvm();
    virtual ~Clvm();

    llvm::GenericValue execute(QString &code, std::vector<llvm::GenericValue> & args,
                               QString func_entry);
    llvm::GenericValue execute2(llvm::Module *mod, QString func_entry);
    virtual void run();

public slots:
    void deleteAsync(QString klass_name, void *obj);

private:
    bool init();
    bool initCompiler();
    bool initExecutionEngine();
    llvm::GenericValue run_module_func(llvm::Module *mod, std::vector<llvm::GenericValue> & args,
                               QString func_entry);

public:
    llvm::GenericValue mretgv;
    llvm::ExecutionEngine *mexe = NULL;
    llvm::EngineBuilder *meb = NULL;
    clang::CompilerInstance *mcis = NULL;
    clang::CompilerInvocation *mciv = NULL;
    clang::driver::Driver *mdrv = NULL;
    ClvmJitListener *mlsner = NULL;
};


#endif /* CLVM_H */
