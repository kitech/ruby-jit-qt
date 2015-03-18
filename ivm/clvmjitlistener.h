#ifndef CLVMJITLISTENER_H
#define CLVMJITLISTENER_H

#include <llvm/ExecutionEngine/JITEventListener.h>

#include "macrolib.h"

class ClvmJitListener : public llvm::JITEventListener,
                        public Singleton<ClvmJitListener>
{
public:
    ClvmJitListener();
    virtual ~ClvmJitListener();

    virtual void NotifyObjectEmitted(const llvm::object::ObjectFile &Obj,
                                     const llvm::RuntimeDyld::LoadedObjectInfo &L);
    virtual void NotifyFreeingObject(const llvm::object::ObjectFile &Obj);


private:
    uint64_t coSize = 0; // created object size
    uint64_t foSize = 0; // freed object size
};

#endif /* CLVMJITLISTENER_H */
