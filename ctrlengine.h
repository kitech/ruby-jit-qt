#ifndef CTRLENGINE_H
#define CTRLENGINE_H

/*
  用于控制调度frontengine查找ast，compilerengine生成ll代码，
  operatorengine生成库方法的调用代码，
  然后传递给clvm执行引擎执行，取回结果。
  使用上，应该在什么位置初始化比较好呢。
*/

#include <QtCore>

namespace clang {
    class CompilerInstance;
    class CompilerInvocation;
    namespace driver {
        class Driver;
    };
};

class FrontEngine;
class CompilerEngine;
class OperatorEngine;
class ClvmEngine;

class CtrlEngine
{
public:
    CtrlEngine();
    ~CtrlEngine();

    // 可能的方法
    void * vm_new(QString klass_name, QVector<QVariant> uargs);
    bool vm_delete();
    bool vm_call();
    bool vm_static_call();

public:
    // 调度组件可能不需要这些组件，只管理调度问题。
    clang::CompilerInstance *mcis = NULL;
    clang::CompilerInvocation *mciv = NULL;
    clang::driver::Driver *mdrv = NULL;

public:
    FrontEngine *mfe = NULL;
    CompilerEngine *mce = NULL;
};

#endif /* CTRLENGINE_H */










