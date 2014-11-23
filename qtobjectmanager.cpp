

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






