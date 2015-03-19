
#include <cassert>

#include "macrolib.h"

// 使用方法演示
class __ASingletonClass : public Singleton<__ASingletonClass>
{
};

// 使用方法演示
class __BSingletonClass : public Singleton<__BSingletonClass>
{
};

// 使用方法演示
class __CSingletonClass : public Singleton<__CSingletonClass>
{
};

// 使用方法演示
class __DSingletonClass : public Singleton<__DSingletonClass>
{

public:
    friend class Singleton;
private:
    __DSingletonClass(int a, char b, const char *c)
    {
    }
    __DSingletonClass()
    {
    }
    void init(int a, char b, const char *c)
    {
    }
};

void __test_macrolib()
{
    auto _ch = __ASingletonClass::inst_simple();
    auto _ch2 = __BSingletonClass::inst_simple();
    auto _ch3 = __CSingletonClass::inst_simple();

    // _ch ==? _ch2 ==? _ch3
    assert((void*)_ch != (void*)_ch2 && (void*)_ch2 != (void*)_ch3);

    auto _vch = __ASingletonClass::inst();
    auto _vch2 = __BSingletonClass::inst();
    auto _vch3 = __CSingletonClass::inst();

    const char *_cv = "cccccc";
    auto _vch4 = __DSingletonClass::inst(5, 'b', (char*)NULL);
    auto _vch5 = __DSingletonClass::inst(5, 'b', _cv);
}










