#include "misc.h"

QString qStr(std::string ss)
{
    return QString::fromStdString(ss);
}

QString qStr(llvm::StringRef sr)
{
    return QString::fromStdString(sr.str());
}

const char *cStr(std::string ss)
{
    return ss.c_str();
}

const char *cStr(llvm::StringRef sr)
{
    return sr.str().c_str();
}

const char *cStr(QString qs)
{
    return qs.toLatin1().data();
}
