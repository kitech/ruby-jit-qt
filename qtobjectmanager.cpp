

#include "qtobjectmanager.h"

// atomic singleton member
namespace _Qom {
    static std::atomic<QtObjectManager*> _inst2(NULL);
    static std::atomic<qint64> _objid2(-1);
    static std::atomic<qint64> _connid(0);
};

QtObjectManager::QtObjectManager()
{
}

QtObjectManager *QtObjectManager::inst()
{
    if (_Qom::_inst2 == NULL) {
        _Qom::_inst2 = new QtObjectManager;
        // init_fmap();
    }
    return _Qom::_inst2;
}

bool QtObjectManager::addObject(RB_VALUE rbobj, void *qtobj)
{
    ObjectInfo *oi = new ObjectInfo;
    oi->objid = ++_Qom::_objid2;
    oi->rbobj = rbobj;
    oi->qtobj = qtobj;

    this->robjs[rbobj] = oi;
    this->qobjs[qtobj] = oi;

    this->jdobjs[rbobj] = qtobj;
    
    return false;
}

bool QtObjectManager::delObject(RB_VALUE rbobj)
{
    ObjectInfo *oi = NULL;

    if (this->jdobjs.contains(rbobj)) {
        this->jdobjs.remove(rbobj);
    }
    
    if (this->robjs.contains(rbobj)) {
        oi = this->robjs.value(rbobj);
        this->robjs.remove(rbobj);
        this->qobjs.remove(oi->qtobj);
        // TODO delete qtobj instance
        free(oi->qtobj);
        delete oi;
        return true;
    }
    
    return false;
}

void *QtObjectManager::getObject(RB_VALUE rbobj)
{
    ObjectInfo *oi = NULL;
    if (this->robjs.contains(rbobj)) {
        oi = this->robjs.value(rbobj);
        return oi->qtobj;
    }
    return NULL;
}

RB_VALUE QtObjectManager::getObject(void *qtobj)
{
    for (auto it = this->jdobjs.begin(); it != this->jdobjs.end(); it++) {
        if (it.value() == qtobj) {
            RB_VALUE rv = it.key();
            return rv;
        }
    }
    return 0;
}

// 这里的信号不能重载。
bool QtObjectManager::addSignals(QString rbklass, QVector<QVariant> sigs)
{

    for (int i = 0; i < sigs.count(); i ++) {
        QString signature = sigs.at(i).toString();
        int pos = signature.indexOf('(');
        QString method = signature.left(pos);

        if (rbsignals.contains(rbklass)) {
            if (rbsignals[rbklass].contains(method)) {
                qDebug()<<"signal existed:"<<method<<signature<<rbsignals[rbklass][method];
            } else {
                rbsignals[rbklass][method] = signature;
            }
        } else {
            QHash<QString, QString> tmp;
            tmp[method] = signature;
            rbsignals[rbklass] = tmp;
        }
    }

    // qDebug()<<rbsignals;
    return true;
}

QString QtObjectManager::getSignature(QString rbklass, QString signal)
{
    if (rbsignals.contains(rbklass)) {
        if (rbsignals[rbklass].contains(signal)) {
            return rbsignals[rbklass][signal];
        }
    }
    return QString();
}

// 与rbconnectrb方法对应
bool QtObjectManager::addConnection(QString rbklass, QString rbsignal, QtObjectManager::RubySlot *rbslot)
{
    QString key = QString("%1->%2").arg(rbklass).arg(rbsignal);
    if (this->rbconnections.contains(key)) {
        this->rbconnections[key].append(rbslot);
    } else {
        QVector<QtObjectManager::RubySlot*> slot = {rbslot};
        this->rbconnections[key] = slot;
    }
    
    return true;
}

QVector<QtObjectManager::RubySlot*>
QtObjectManager::getConnections(QString rbklass, QString rbsignal)
{
    QVector<QtObjectManager::RubySlot*> conns;
    QString key = QString("%1->%2").arg(rbklass).arg(rbsignal);

    if (this->rbconnections.contains(key)) {
        return this->rbconnections.value(key);
    }
    
    return conns;
}

// 为什么在QHash中还是不能用呢。
inline uint qHash(const uint128_t & key, uint seed) Q_DECL_NOTHROW
{
    uint h = 0;

    quint64 hbit, lbit;
    memcpy(&hbit, &key, 8);
    memcpy(&lbit, ((char*)&key)+8, 8);

    uint hh,hl;
    hh = qHash(hbit);
    hl = qHash(lbit);

    quint64 tkey = hh;
    tkey = tkey << 32;
    hbit = tkey | hl;
    h = qHash(hbit);
    
    return h;
}

bool QtObjectManager::addConnection(RB_VALUE rbobj, QString rbsignal, RubySlot *rbslot)
{
    QPair<RB_VALUE, QString> ckey = {rbobj, rbsignal};
    if (this->rbconnections4.contains(ckey)) {
        this->rbconnections4[ckey].append(rbslot);
    } else {
        QVector<QtObjectManager::RubySlot*> slot = {rbslot};
        this->rbconnections4[ckey] = slot;
    }

    /// 
    RB_VALUE ikey = rbobj;
    if (this->rbconnections3.contains(ikey)) {
        if (this->rbconnections3[ikey].contains(rbsignal)) {
            this->rbconnections3[ikey][rbsignal].append(rbslot);
        } else {
            QVector<QtObjectManager::RubySlot*> slot = {rbslot};            
            this->rbconnections3[ikey][rbsignal] = slot;
        }
    } else {
        QVector<QtObjectManager::RubySlot*> slot = {rbslot};
        QHash<QString, QVector<QtObjectManager::RubySlot*> > hslots;
        hslots[rbsignal] = slot;
        this->rbconnections3[ikey] = hslots;
    }
    
    return true;
}

QVector<QtObjectManager::RubySlot*>
QtObjectManager::getConnections(RB_VALUE rbobj, QString rbsignal)
{
    QPair<RB_VALUE, QString> ckey = {rbobj, rbsignal};
    if (this->rbconnections4.contains(ckey)) {
        return this->rbconnections4[ckey];
    }

    // way 2
    QVector<QtObjectManager::RubySlot*> conns;
    RB_VALUE ikey = rbobj;
    
    if (this->rbconnections3.contains(ikey)) {
        if (this->rbconnections3[ikey].contains(rbsignal)) {
            conns = this->rbconnections3[ikey][rbsignal];
        }
    }
    
    return conns;
}

bool QtObjectManager::removeConnection(RB_VALUE rbobj, QString rbsignal,
                                       const QtObjectManager::RubySlot *rbslot)
{    
    QVector<QtObjectManager::RubySlot*> conns;
    conns = this->getConnections(rbobj, rbsignal);
    if (conns.count() == 0) {
        qDebug()<<"no rbconn:"<<rbobj<<rbsignal;
        return true;
    }

    int rmcnt = 0;    
    QPair<RB_VALUE, QString> ckey = {rbobj, rbsignal};
    RB_VALUE ikey = rbobj;
    for (int i = conns.count()-1; i >= 0; i--) {
        auto slot = conns.at(i);        
        if (
            // 删除该signal所有的slot            
            rbslot == NULL ||
            // 删除匹配的slot
            (slot->receiver == rbslot->receiver && slot->slot == rbslot->slot)) {
            this->rbconnections3[ikey][rbsignal].remove(i);
            this->rbconnections4[ckey].remove(i);
            delete slot;
            rmcnt ++;
        }
    }
    qDebug()<<"removed conn3:"<<rmcnt<<rbslot;
    
    return true;
}

void QtObjectManager::testParser()
{
    // get_class_method("abcdef.h");
}

void QtObjectManager::testIR()
{
    // run_code_jit();
}

QDebug &qodebug(QDebug &dbg, void*obj, QString klass)
{
    // _Zls6QDebugRK9QBitArray
    QString symtpl = "_Zls6QDebugRK%1%2";
    QString symname = symtpl.arg(klass.length()).arg(klass);

    
    
    return dbg.space();
}

/////////////////////
////
/////////////////////
// qint64 ConnectProxy::connid = 1;
ConnectProxy::ConnectProxy()
    : QObject()
{
}

qint64 ConnectProxy::addConnection(QMetaObject::Connection conn)
{
    qint64 nid = _Qom::_connid++;
    conns[nid] = conn;
    
    return nid;
}






