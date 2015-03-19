#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include "modulemanager.h"

bool ModuleManager::add(QString name, llvm::Module* mod)
{
    if (contains(name)) return true;

    //////
    modules[name] = mod;

    std::unique_ptr<llvm::Module> tmod(mod);
    mee->addModule(std::move(tmod));
    
    return true;
}

bool ModuleManager::remove(QString name)
{
    llvm::Module* mod = modules.value(name);
    modules.remove(name);

    if (mod != NULL) {
        mee->removeModule(mod);
    }
    
    return true;
}

bool ModuleManager::contains(QString name)
{
    return modules.contains(name);
    return true;
}

llvm::Module *ModuleManager::get(QString name)
{
    return modules.value(name);
}
