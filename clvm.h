#ifndef CLVM_H
#define CLVM_H

#include <QtCore>

namespace llvm {
    struct GenericValue;
    class ExecutionEngine;
    class EngineBuilder;
};

llvm::GenericValue vm_execute(QString code, QVector<llvm::GenericValue> &envp);

class Clvm 
{
public:
    Clvm();
    virtual ~Clvm();

public:
    llvm::GenericValue m_retgv;
    
};


#endif /* CLVM_H */
