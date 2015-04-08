#ifndef MISC_H
#define MISC_H

// qt llvm std:: string helper
#include <QString>
#include <llvm/ADT/StringRef.h>

QString qStr(std::string ss);
QString qStr(llvm::StringRef sr);

const char *cStr(std::string ss);
const char *cStr(llvm::StringRef sr);
const char *cStr(QString qs);

// QString to llvm:StringRef???

const static QString qtmod_prefix = "qtmod_";
const static QString remod_prefix = "remod_";



#endif /* MISC_H */
