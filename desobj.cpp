
#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/ELFObjectFile.h>
#include <llvm/Support/DynamicLibrary.h>
#include <QtCore>

#include "desobj.h"


using namespace llvm;
using namespace llvm::object;
using namespace llvm::sys;

void test_desobj()
{
    ErrorOr<ObjectFile *> objfile = ObjectFile::createObjectFile("/usr/lib/libQtCore.so");
    ObjectFile *pobj = objfile.get();

    qDebug()<<"heheho"<<pobj->getFileFormatName().data();
    int i = 0;

    DynamicLibrary dlib = DynamicLibrary::getPermanentLibrary("/usr/lib/libQt5Core.so");
    qDebug()<<dlib.isValid();
    void *addr = dlib.SearchForAddressOfSymbol("_ZN7QString4chopEi");
    qDebug()<<"addr:"<<addr;
}
