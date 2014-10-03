#ifndef CLVM_LETACY_H
#define CLVM_LETACY_H


#include <llvm/ExecutionEngine/GenericValue.h>

class QString;
template <typename T> class QVector;

llvm::GenericValue vm_execute(QString code, QVector<llvm::GenericValue> &envp);

#endif /* CLVM_LETACY_H */













