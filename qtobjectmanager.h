#ifndef QTOBJECTMANAGER_H
#define QTOBJECTMANAGER_H

#include <QtCore>

typedef quint64 RB_VALUE;
typedef quint64 RB_ID;

// Qt Object Manager
class QtObjectManager
{
public:
    static QtObjectManager *inst();
    struct ObjectInfo {
        qint64 objid = 0;
        RB_VALUE rbobj = 0;  // VALUE type
        void *qtobj = NULL;  // Qxxx type
    };
    
    struct RubySlot {
        RB_VALUE receiver = 0;
        RB_ID slot = 0;
        QString raw_slot;
        RB_VALUE sender = 0;
        RB_ID signal = 0;
        QString raw_signal;
    };
    
    QHash<quint64, void *> jdobjs; // jit'ed objects
    QHash<quint64, QObject *> objs;
    // QHash<QString, const QMetaObject *> metas;
    QHash<void *, ObjectInfo*> qobjs; // qtobject => ObjectInfo
    QHash<quint64, ObjectInfo*> robjs; // rb_hash(rb_object) => ObjectInfo

public:
    bool addObject(RB_VALUE rbobj, void *qtobj);
    bool delObject(RB_VALUE rbobj);
    void *getObject(RB_VALUE rbobj);
    RB_VALUE *getObject(void *qtobj);
    
public:
    void testParser();
    void testIR();

    void **getfp(QString klass, QString method);

private:
    QtObjectManager();
    static QtObjectManager *_inst;
    static qint64 _objid;
};
typedef QtObjectManager Qom; // for simple

class ConnectProxy : public QObject
{
    Q_OBJECT;
public:
    ConnectProxy();
    virtual ~ConnectProxy() {}

public slots:
    void proxycall();
    void proxycall(int);
    void proxycall(char);
    void proxycall(void *);

public:
    qint64 addConnection(QMetaObject::Connection conn);
public:
    QHash<qint64, QMetaObject::Connection> conns;
    static qint64 connid;
    //
    int argc;
    RB_VALUE *argv;
    RB_VALUE self;
    //
    QObject *osender;
    QString osignal;
    QObject *oreceiver;
    QString oslot;
};

#endif /* QTOBJECTMANAGER_H */
















