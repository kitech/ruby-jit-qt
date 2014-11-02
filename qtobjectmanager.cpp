

#include "qtobjectmanager.h"

QtObjectManager *QtObjectManager::_inst = NULL;
qint64 QtObjectManager::_objid = -1;

QtObjectManager::QtObjectManager()
{
}

QtObjectManager *QtObjectManager::inst()
{
    if (QtObjectManager::_inst == NULL) {
        QtObjectManager::_inst = new QtObjectManager;
        // init_fmap();
    }
    return QtObjectManager::_inst;
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
qint64 ConnectProxy::connid = 1;
ConnectProxy::ConnectProxy()
    : QObject()
{
}

qint64 ConnectProxy::addConnection(QMetaObject::Connection conn)
{
    qint64 nid = connid++;
    conns[nid] = conn;
    
    return nid;
}






