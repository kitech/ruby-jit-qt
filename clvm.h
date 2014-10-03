#ifndef CLVM_H
#define CLVM_H

#include <llvm/ExecutionEngine/GenericValue.h>

#include <QtCore>

namespace llvm {
    struct GenericValue;
    class ExecutionEngine;
    class EngineBuilder;
};

namespace clang {
    class CompilerInvocation;
    class CompilerInstance;

    namespace driver {
        class Driver;
    };
};

llvm::GenericValue vm_execute(QString code, QVector<llvm::GenericValue> &envp);

class Clvm : public QThread
{
    Q_OBJECT;
public:
    Clvm();
    virtual ~Clvm();

    llvm::GenericValue execute(QString &code, std::vector<llvm::GenericValue> & args,
                               QString func_entry);
    virtual void run();

public slots:
    void deleteAsync(QString klass_name, void *obj);

private:
    bool init();
    bool initCompiler();
    bool initExecutionEngine();

public:
    llvm::GenericValue mretgv;
    llvm::ExecutionEngine *mexe = NULL;
    llvm::EngineBuilder *meb = NULL;
    clang::CompilerInstance *mcis = NULL;
    clang::CompilerInvocation *mciv = NULL;
    clang::driver::Driver *mdrv = NULL;
};


#endif /* CLVM_H */












