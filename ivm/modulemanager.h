#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include <QtCore>

#include "macrolib.h"

namespace llvm {
    class Module;
    class ExecutionEngine;
}

/*
  管理传递给ExecutionEngine的llvm::Module对象
  实现llvm::Module的缓存，解决重复生成llvm::Module的问题，优化IR执行过程。

  该类管理的Module的名字必须与管理时使用的名字相同。
  可以考虑使用生成这个Module的symbol名当作Module的名字。

  这个类应该是单例的。是否要考虑多线程的使用呢？

  注意，这个类中的Module对象可能处于已销毁状态，可能是野指针，需要改进。
 */

class ModuleManager : public Singleton<ModuleManager>
{
public:
    ModuleManager(/*llvm::ExecutionEngine* ee*/) {}
    virtual ~ModuleManager() {}
    /*
    ModuleManager* init(llvm::ExecutionEngine* ee) {
        qDebug()<<"eeeeee:"<<ee;
        mee = ee;
        return this;
    }
    */
    
    bool add(QString name, llvm::Module* mod);
    bool remove(QString name);
    bool contains(QString name);
    llvm::Module *get(QString name);
    int size() { return modules.size(); }
    const QStringList keys() { return modules.keys(); }
    
public:
    QHash<QString, llvm::Module*> modules; // called symbol name => Module
    // QHash<QString, llvm::Module*> entries; // entries symbol name => Module
    llvm::ExecutionEngine *mee = NULL;
};

#endif /* MODULEMANAGER_H */
