#ifndef RUBY_CXX_H
#define RUBY_CXX_H

#include <pthread.h>

#include <QString>
// #include "fix_clang_undef_eai.h"

extern "C" {
    #include <ruby.h>
}; /* satisfy cc-mode */

#include "macrolib.h"

/*
  实现ruby.h的C++封装
  auto rbx = RubyCXX::inst();
*/
class RubyCXX : public Singleton<RubyCXX>
{
    // DECL_SINGLETON_CLASS(RubyCXX);
public:
    // static RubyCXX *inst();
    virtual ~RubyCXX();

    inline int type(VALUE v) { return rb_type(v); }
    inline bool isObject(VALUE v) { return rb_type(v) == T_OBJECT;}
    inline bool isSymbol(VALUE v) { return rb_type(v) == T_SYMBOL;}
    inline bool isClass(VALUE v) { return rb_type(v) == T_CLASS;}
    QString className(VALUE v);
    QString rbClassName(VALUE v);
    QString qtClassname(VALUE v);

    /*
private:
    RubyCXX();
    static RubyCXX *m_inst; // = NULL;
    static pthread_mutex_t m_mutex;// = PTHREAD_MUTEX_INITIALIZER;
    */
};

#endif /* RUBY_CXX_H */
