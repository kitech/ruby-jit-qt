#include <QStringList>

#include "ruby_cxx.h"

/*
RubyCXX *RubyCXX::m_inst = NULL;
pthread_mutex_t RubyCXX::m_mutex = PTHREAD_MUTEX_INITIALIZER;

RubyCXX *RubyCXX::inst()
{
    if (RubyCXX::m_inst == NULL) {
        pthread_mutex_lock(&RubyCXX::m_mutex);
        if (RubyCXX::m_inst == NULL) {
            RubyCXX::m_inst = new RubyCXX();
        }
        pthread_mutex_unlock(&RubyCXX::m_mutex);
    }
    return RubyCXX::m_inst;
}

RubyCXX::RubyCXX()
{
}
*/

// DEFINE_SINGLETON_CLASS(RubyCXX);

/*
// test singleton class 
StClass *_ch = StClass::inst();
RubyCXX *_rbx = RubyCXX::inst();
*/

RubyCXX::~RubyCXX()
{
}

QString RubyCXX::className(VALUE v)
{
    if (rb_type(v) != T_OBJECT && rb_type(v) != T_CLASS) {
        return QString();
    }

    VALUE self = v;
    if (TYPE(self) == T_CLASS) {
        QString klass_name = rb_class2name(self);
        return klass_name;
    }
    // T_OBJECT
    else {
        QString klass_name = rb_class2name(RBASIC_CLASS(self));
        return klass_name;
    }
    
    Q_ASSERT(1==2);
}

QString RubyCXX::rbClassName(VALUE v)
{
    VALUE self = v;
    if (TYPE(self) == T_CLASS) {
        QString klass_name = rb_class2name(self);
        if (klass_name.startsWith("Qt5::Q")) return QString();
        else return klass_name;
    }
    // T_OBJECT
    else {
        QString klass_name = rb_class2name(RBASIC_CLASS(self));
        if (klass_name.startsWith("Qt5::Q")) return QString();
        else return klass_name;
    }
}

/*
// 这应该叫统一的方法，获取qt class名称
// get_qt_class(VALUE self)
// 判断是否是qt类，如果不是，很可能是在Ruby中继承Qt类
// 如果是继承类，取到需要实例化的Qt类名
TODO 还需要考虑继承层次大于2的情况
@param self T_OBJECT | T_CLASS
 */
QString RubyCXX::qtClassname(VALUE v)
{
    VALUE self = v;
    if (TYPE(self) == T_CLASS) {
        QString klass_name = rb_class2name(self);
        if (klass_name.startsWith("Qt5::Q")) return klass_name.split("::").at(1);

        // 如果是继承类，取到需要实例化的Qt类名
        VALUE pcls = RCLASS_SUPER(self);
        klass_name = rb_class2name(pcls);
        return klass_name.split("::").at(1);
    }
    // T_OBJECT
    else {
        QString klass_name = rb_class2name(RBASIC_CLASS(self));
        if (klass_name.startsWith("Qt5::Q")) return klass_name.split("::").at(1);

        // 如果是继承类，取到需要实例化的Qt类名
        VALUE pcls = RCLASS_SUPER(RBASIC_CLASS(self));
        klass_name = rb_class2name(pcls);
        return klass_name.split("::").at(1);
    }    
}




