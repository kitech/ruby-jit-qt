
#include <llvm/IR/Module.h>

#include "codeunit.h"

CodeUnit::CodeUnit(llvm::Module *qtmod, llvm::Module *remod)
{
    this->qtmod = qtmod;
    qtmod->getName();
    this->remod = remod;
    remod->getName();
}

