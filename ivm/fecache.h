#ifndef FECACHE_H
#define FECACHE_H

#include <QtCore>

namespace clang {
    class CXXRecordDecl;
}

class FECache
{
public:
    QHash<QString, clang::CXXRecordDecl*> mCxxRecs;
};


#endif /* FECACHE_H */
