

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






