#ifndef _CPP_RUBY_HPP
#define _CPP_RUBY_HPP

#include "fix_clang_undef_eai.h"

extern "C" {
    #include <ruby.h>
}; /* satisfy cc-mode */

/*
  实现ruby.h的C++封装      
*/
class CppRuby
{
public:
    CppRuby();
    virtual ~CppRuby();

private:
    
};

#endif /* _CPP_RUBY_HPP */


