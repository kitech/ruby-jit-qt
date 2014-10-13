#ifndef CTRLENGINE_H
#define CTRLENGINE_H

/*
  用于控制调度frontengine查找ast，compilerengine生成ll代码，
  传递给clvm执行引擎执行，取回结果。
 */

#include <QtCore>

namespace clang {
    class CompilerInstance;
    class CompilerInvocation;
    namespace driver {
        class Driver;
    };
};

class CtrlEngine
{
public:
    CtrlEngine();
    ~CtrlEngine();

public:
    clang::CompilerInstance *mcis = NULL;
    clang::CompilerInvocation *mciv = NULL;
    clang::driver::Driver *mdrv = NULL;
};

#endif /* CTRLENGINE_H */
