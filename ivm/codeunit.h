#ifndef CODEUNIT_H
#define CODEUNIT_H

#include <QtCore>

namespace llvm {
    class Module;
}


/*
  拆分qt代码Module和入口代码Module，
  准备实现qt代码Module的重复使用。
 */

// LiveCodeUnit || JitCodeUnit
class CodeUnit
{
public:
    CodeUnit(llvm::Module *qtmod, llvm::Module *remod);

public:
    llvm::Module *qtmod = NULL;  // qt ir code module
    llvm::Module *remod = NULL;  // run entry module or wrapper module
    QString rename; // entry name
};

#endif /* CODEUNIT_H */
