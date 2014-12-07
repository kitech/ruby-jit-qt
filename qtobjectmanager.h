#ifndef QTOBJECTMANAGER_H
#define QTOBJECTMANAGER_H

#include "fix_clang_undef_eai.h"

#include <functional>
#include <atomic>
#include <QtCore>

typedef quint64 RB_VALUE;
typedef quint64 RB_ID;
#ifndef uint128_t
#define uint128_t unsigned __int128
#endif

// uint qHash(uint128_t key, uint seed = 0) Q_DECL_NOTHROW;

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
        RB_VALUE vslot = 0;
        QString raw_slot;
        RB_VALUE sender = 0;
        RB_ID signal = 0;
        RB_VALUE vsignal = 0;        
        QString raw_signal;
    };
        
public:
    QHash<quint64, QObject *> objs;
    // QHash<QString, const QMetaObject *> metas;

private:
    QHash<RB_VALUE, void *> jdobjs; // jit'ed objects
private:
    QHash<void *, ObjectInfo*> qobjs; // qtobject => ObjectInfo
    QHash<RB_VALUE, ObjectInfo*> robjs; // rb_object => ObjectInfo
    // QHash<classname, QHash<signame, signature> >
    QHash<QString, QHash<QString, QString> > rbsignals;
    // QHash<classname_sigal_method, QVector<RubySlot*> >
    QHash<QString, QVector<RubySlot*> > rbconnections;
    // QHash<rbobj << 64 & rbmethod, QVector<RubySlot*> >
    unsigned __int128 v128;
    QHash<uint128_t, QVector<RubySlot*> > rbconnections2;
    QHash<QPair<RB_VALUE, QString>, QVector<RubySlot*> > rbconnections4;
    QHash<RB_VALUE, QHash<QString, QVector<RubySlot*> > > rbconnections3;
    
    
public:
    bool addObject(RB_VALUE rbobj, void *qtobj);
    bool delObject(RB_VALUE rbobj);
    void *getObject(RB_VALUE rbobj);
    RB_VALUE getObject(void *qtobj);

    // 与class.signals()对应
    bool addSignals(QString rbklass, QVector<QVariant> sigs);
    QString getSignature(QString rbklass, QString signal);

    // 与rbconnectrb方法对应
    // depcreated
    bool addConnection(QString rbklass, QString rbsignal, RubySlot *rbslot);
    QVector<RubySlot*> getConnections(QString rbklass, QString rbsignal);
    // 与rbconnectrb方法对应    
    bool addConnection(RB_VALUE rbobj, QString rbsignal, RubySlot *rbslot);
    QVector<RubySlot*> getConnections(RB_VALUE rbobj, QString rbsignal);
    bool removeConnection(RB_VALUE rbobj, QString rbsignal, const RubySlot *rbslot);
    
public:
    void testParser();
    void testIR();

    void **getfp(QString klass, QString method);

private:
    QtObjectManager();
};
typedef QtObjectManager Qom; // for simple

QDebug &qodebug(QDebug &dbg, void*obj, QString klass);

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
    // static qint64 connid;
    //
    int argc;
    RB_VALUE *argv;
    RB_VALUE self;
    //
    QObject *osender;
    QString osignal;
    // QObject *oreceiver;
    RB_VALUE oreceiver;
    QString oslot;
    std::function<void()> funtor;
};

#endif /* QTOBJECTMANAGER_H */
















