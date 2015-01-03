#ifndef MACROLIB_H
#define MACROLIB_H

#include <pthread.h>

// 生成无参数单例类
#define DECL_SINGLETON_CLASS(klass)                      \
    private:                                             \
        klass();                                         \
        static klass *m_inst;                            \
        static pthread_mutex_t m_mutex;                  \
    public:                                              \
        static klass *inst();

#define DEFINE_SINGLETON_CLASS(klass)           \
    klass *klass::m_inst = NULL;                \
    pthread_mutex_t klass::m_mutex = PTHREAD_MUTEX_INITIALIZER; \
    klass::klass() {}                                           \
    klass *klass::inst()                                        \
    {                                                           \
        if (klass::m_inst == NULL) {                            \
            pthread_mutex_lock(&klass::m_mutex);                \
            if (klass::m_inst == NULL) {                        \
                klass::m_inst = new klass();                    \
            }                                                   \
            pthread_mutex_unlock(&klass::m_mutex);              \
        }                                                       \
        return klass::m_inst;                                   \
    }

// 模板方式实现
// 支持有任意参数的构造函数的单实例类
template<typename T>
class Singleton
{
protected:
    // TODO 怎么初始化静态变量呢？
    static T *m_inst;
    static pthread_mutex_t m_mutex;

public:
    virtual ~Singleton()
    {
        delete m_inst;
        m_inst = NULL;
    }
    
public:
    [[deprecated("")]]
    static T *inst_simple()
    {
        if (Singleton::m_inst == NULL) {
            pthread_mutex_lock(&Singleton::m_mutex);
            if (Singleton::m_inst == NULL) {
                Singleton::m_inst = new T();
            }
            pthread_mutex_unlock(&Singleton::m_mutex);
        }
        return Singleton::m_inst;
    }

    // need c++11
    // inst() with varidic template argument.
    template<typename... Arguments>
    static T *inst(Arguments... parameters)
    {
        if (Singleton<T>::m_inst == NULL) {
            pthread_mutex_lock(&Singleton<T>::m_mutex);
            if (Singleton<T>::m_inst == NULL) {
                Singleton<T>::m_inst = new T(parameters...);
            }
            pthread_mutex_unlock(&Singleton<T>::m_mutex);
        }
        
        return Singleton<T>::m_inst;
    }

public: // needed for inst()
    friend class Singleton; // friend 关系不会继承，子类中还需要这个表达式。
    friend T; // 没有什么影响
};

// 模板类静态变量初始化。（必须放在头文件中啊）
template<typename T> T *Singleton<T>::m_inst = NULL;
template<typename T> pthread_mutex_t Singleton<T>::m_mutex = PTHREAD_MUTEX_INITIALIZER;

#endif /* MACROLIB_H */



