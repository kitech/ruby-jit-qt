#ifndef NAMELOOKUP_H
#define NAMELOOKUP_H

#include "fix_clang_undef_eai.h"

#include <QtCore>

namespace clang {
    class CXXRecordDecl;
};
class FrontEngine;

bool nlu_find_method(FrontEngine *fe, QString method, clang::CXXRecordDecl *rd);

class NameLookup
{
public:
    
};

#endif /* NAMELOOKUP_H */

