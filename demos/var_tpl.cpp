#include <QtCore>

#include <functional>


QString a5;
auto fffffffffffffff() -> decltype(a5.clear()) {
    QString a;
    return a.clear();
}

template<typename T, typename... Args>
        static QVariant call(T *ci, char *method, Args... args)
        {
            if (strcmp(method, "append") == 0) {
                auto rv = ci->append(args...); // error
                qDebug()<<"result:"<<rv;

                return QVariant(rv);
            }

            if (strcmp(method, "arg") == 0) {
                return QVariant(1);
            }

            return QVariant();
        }

// 在C++中不允许将类成员方法指针转成void*
#include <dlfcn.h>
template<typename T>
static void * voidify(T method)
{
// asm ("movq %rdi, %rax"); // should work on x86_64 ABI compliant platformsyyyy
// asm (); // should work on x86 ABI compliant platformsxxxx
// asm("movl %esp, %eax");
}

template<typename T>
const char* getMethodName(T method)
{
    Dl_info info;
    if (dladdr(voidify(method), &info))
        return info.dli_sname;
    return "";
}

int main(int argc, char **argv)
{
    QString b;
    QVariant r1 = ::call<decltype(b)>(&b, "append", "cdcc");
    // QVariant r2 = ::call<decltype(b)>(&b, "append", 12345);
    // QVariant r3 = ::call<decltype(b)>(&b, "append", 'h');
    // QVariant rx = ::call<decltype(b)>(&b, "append", 'h');

    // QVariant r4 = ::call<decltype(b)>(&b, "appendccc", 'h');

    // QVariant r5 = ::call<decltype(b)>(&b, "arg", 123);

    // qDebug()<<r1<<r2<<r3<<r4<<r5;

    /*
    char *a = (char*)calloc(1, sizeof(QString));
    memset(a, 0, sizeof(QString));
    QString *s = (QString*)a;
    qDebug()<<s->length();
    */

    qDebug()<<"QString size:"<<sizeof(QString)<<sizeof(QByteArray)<<sizeof(QVariant);

    // qDebug()<<getMethodName(&QString::length);


    return 0;
};
