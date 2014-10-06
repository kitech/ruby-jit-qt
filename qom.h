#ifndef QOM_H
#define QOM_H

#include <QtCore>


class Qom 
{
public:
    static Qom *inst();
    QHash<quint64, void *> jdobjs; // jit'ed objects
    QHash<quint64, QObject *> objs;
    QHash<QString, const QMetaObject *> metas;


    void testParser();
    void testIR();

    void **getfp(QString klass, QString method);

private:
    Qom();
    static Qom *_inst;
};


#endif /* QOM_H */
















