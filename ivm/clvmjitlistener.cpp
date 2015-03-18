
#include <QtCore>

#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/ELFObjectFile.h>

#include "clvmjitlistener.h"

ClvmJitListener::ClvmJitListener() : llvm::JITEventListener()
{
}

ClvmJitListener::~ClvmJitListener()
{
}

static void dumpObjectFile(const llvm::object::ObjectFile &Obj)
{
    const llvm::object::ObjectFile *pObj = &Obj;
    
    qDebug()<<Obj.getFileFormatName().str().c_str();
    qDebug()<<Obj.getFileName().str().c_str()<<Obj.getData().str().length();
    qDebug()<<Obj.getType()<<(llvm::isa<llvm::object::ELFObjectFileBase>(pObj));
    // <<(llvm::isa<llvm::object::ELFObjectFile<???> >(pObj));

    for (auto sym : Obj.symbols()) {
        std::string name;
        llvm::StringRef rname(name);
        uint64_t sz = 0;
        sym.getName(rname);
        sym.getSize(sz);
        qDebug()<<"sym:"<<&sym<<sz<<rname.str().c_str();
    }

    for (auto sec : Obj.sections()) {
        std::string name;
        llvm::StringRef rname(name);
        uint64_t sz = 0;
        sec.getName(rname);
        sz = sec.getSize();
        qDebug()<<"sec:"<<&sec<<sz<<rname.str().c_str();
    }
}

void ClvmJitListener::NotifyObjectEmitted(const llvm::object::ObjectFile &Obj,
                                          const llvm::RuntimeDyld::LoadedObjectInfo &L)
{
    coSize += Obj.getData().str().length();    
    qDebug()<<&Obj<<&L<<coSize<<foSize;
    dumpObjectFile(Obj);
}

void ClvmJitListener::NotifyFreeingObject(const llvm::object::ObjectFile &Obj)
{
    foSize += Obj.getData().str().length();
    qDebug()<<&Obj<<coSize<<foSize;
    dumpObjectFile(Obj);    
}


