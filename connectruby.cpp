#include <cassert>

#include <private/qobject_p.h>

#include "qtobjectmanager.h"
#include "marshallruby.h"
#include "connectruby.h"


void ConnectAny::call_ruby(int argc, const VALUE *argv)
{
    if (m_conn_type == CONN_RUBY_TO_RUBY) {
        RubyConnectRuby *r2r = (RubyConnectRuby*)this;
        VALUE rbret = rb_funcall3(r2r->m_receiver, r2r->m_slot_id, argc, argv);
    }
    else if (m_conn_type == CONN_QT_TO_RUBY) {
        QtConnectRuby *q2r = (QtConnectRuby*)this;
        VALUE rbret = rb_funcall3(q2r->m_receiver, q2r->m_slot_id, argc, argv);
    }
}

void ConnectAny::call_qt(int argc, const VALUE *argv)
{
    
}

void RubyConnectRuby::call(int argc, const VALUE *argv)
{
    this->call_ruby(argc, argv);
}

bool RubyConnectRuby::eventFilter(QObject * watched, QEvent * event)
{
    if (event->type() == QEvent::MetaCall) {
        qDebug()<<"evvvvvvvvvvvvtttt"<<watched<<event;
        QMetaCallEvent *evt = (QMetaCallEvent*)event;
        qDebug()<<"meta cal evt:"<<evt<<evt->args();
        this->mcevt = evt;
    }
    return false;    
}

void RubyConnectRuby::router(int argc, VALUE *argv, VALUE obj)
{
    qDebug()<<"invoked..."<<argc<<argv<<obj;
    this->call_ruby(argc, argv);
}


void QtConnectRuby::call(int argc, const VALUE *argv)
{
    this->call_ruby(argc, argv);
}

bool QtConnectRuby::eventFilter(QObject * watched, QEvent * event)
{
    if (event->type() == QEvent::MetaCall) {
        qDebug()<<"evvvvvvvvvvvvtttt"<<watched<<event;
        QMetaCallEvent *evt = (QMetaCallEvent*)event;
        qDebug()<<"meta cal evt:"<<evt<<evt->args();
        this->mcevt = evt;
    }
    return false;
}

void QtConnectRuby::router()
{
    qDebug()<<"hereeeeeeee"<<this->mcevt;
    qDebug()<<this->m_sender<<this->m_signal<<this->m_receiver<<this->m_slot;
    
    assert(this->mcevt != NULL);
    assert(this->m_sender == sender());

    // 查找sender的signature，确定参数个数
    QString rawsig = this->m_signal.right(this->m_signal.length()-1);
    QByteArray normsig = QMetaObject::normalizedSignature(rawsig.toLatin1().data());
    const QMetaObject *mo = this->m_sender->metaObject();
    int idx = mo->indexOfSignal(normsig.data());
    QMetaMethod mm = mo->method(idx);
    qDebug()<<normsig<<idx;
    qDebug()<<mm.parameterCount();

    if (idx == -1) {
        qDebug()<<"can not get a method";
        assert(idx != -1);
        return;
    }

    int argc = mm.parameterCount();
    VALUE argv[10] = {0};

    void **args = mcevt->args();
    for (int i = 1; i < argc+1; i++) {
        int pty = mm.parameterType(i-1);
        void *arg = args[i];

        argv[i-1] = MarshallRuby::Variant2VALUE(arg, pty);
    }

    //
    // qDebug()<<TYPE(this->m_slot);
    // ID call_id = rb_intern("call");
    // VALUE rbret = rb_funcall3(this->m_slot, call_id, argc, argv);
    this->call(argc, argv);
}

void RubyConnectQt::call(int argc, const VALUE *argv)
{
    qDebug()<<argc<<"not impled";
}

void QtConnectQt::call(int argc, const VALUE *argv)
{
    qDebug()<<argc<<"not impled";
}

bool QtConnectQt::eventFilter(QObject * watched, QEvent * event)
{
    if (event->type() == QEvent::MetaCall) {
        qDebug()<<"evvvvvvvvvvvvtttt"<<watched<<event;
        QMetaCallEvent *evt = (QMetaCallEvent*)event;
        qDebug()<<"meta cal evt:"<<evt<<evt->args();
        this->mcevt = evt;
    }
    return false;
}

void QtConnectQt::router()
{
    qDebug()<<"hereeeeeeee"<<this->mcevt;
    qDebug()<<this->m_sender<<this->m_signal<<this->m_receiver<<this->m_slot;
    
    assert(this->mcevt != NULL);
    assert(this->m_sender == sender());

    qDebug()<<"meta calling qt slot...";
    this->mcevt->placeMetaCall(this->m_receiver); // faild
    // int ret = QMetaObject::metacall(this->m_receiver, QMetaObject::InvokeMetaMethod, mcevt->id(), mcevt->args());
    // qDebug()<<"meta call:"<<ret;
}

////////////////////////
////
// static
ConnectAny *ConnectFactory::create(int argc, VALUE *argv, VALUE obj)
{
    ConnectAny *conn = NULL;

    QString method_name = rb_id2name(SYM2ID(argv[0]));
    qDebug()<<method_name<<argc;

    if (method_name == "qtconnectrb") {
        QObject *sender = (QObject*)Qom::inst()->getObject(argv[1]);
        QVariant vsignal = MarshallRuby::VALUE2Variant(argv[2]);
        QString signal = vsignal.toString();
        QString rsignal = QString("2%1").arg(signal); // real signal

        QtConnectRuby *xconn = NULL;
        if (argc == 4) {
            xconn = new QtConnectRuby(sender, rsignal, argv[3]);
        } else if (argc == 5) {
            xconn = new QtConnectRuby(sender, rsignal, argv[3], argv[4]);
        }
        
        QMetaObject::Connection qtconn =
            QObject::connect(sender, rsignal.toLatin1().data(), xconn, SLOT(router()), Qt::QueuedConnection);
        xconn->installEventFilter(xconn); // ahaha, niubeee

        xconn->qtconn = qtconn;
        conn = xconn;
    }

    else if (method_name == "qtconnectqt") {
        QObject *sender = (QObject*)Qom::inst()->getObject(argv[1]);
        QVariant vsignal = MarshallRuby::VALUE2Variant(argv[2]);
        QString signal = vsignal.toString();
        QString rsignal = QString("2%1").arg(signal); // real signal

        QObject *receiver = (QObject*)Qom::inst()->getObject(argv[3]);
        QVariant vslot = MarshallRuby::VALUE2Variant(argv[4]);
        QString slot = vslot.toString();
        QString rslot = QString("1%1").arg(slot);

        QtConnectQt *xconn = new QtConnectQt(sender, rsignal, receiver, rslot);
        QMetaObject::Connection qtconn =
            // QObject::connect(sender, rsignal.toLatin1().data(), xconn, SLOT(router()), Qt::QueuedConnection);
            QObject::connect(sender, rsignal.toLatin1().data(), receiver, rslot.toLatin1().data(), Qt::QueuedConnection);
        xconn->installEventFilter(xconn);

        xconn->qtconn = qtconn;
        conn = xconn;
    }

    else if (method_name == "rbconnectrb") {
        conn = ConnectFactory::create_rbconnectrb(argc, argv, obj);
        assert(conn != NULL);
    }

    else if (method_name == "rbconnectqt") {

    }
    
    /*
    // Qt5::connectrb    
    if (method_name == "connectrb") {
        qDebug()<<"got ittttttt."<<method_name;
        return x_Qt_connectrb(argc, argv, self);
    } 
    // Qt5::rbconnectrb
    else if (method_name == "rbconnectrb") {
        qDebug()<<"got ittttttt."<<method_name;
        return x_Qt_rbconnectrb(argc, argv, self);
    }
    // Qt5::rbdisconnectrb
    else if (method_name == "rbdisconnectrb") {
        qDebug()<<"got ittttttt."<<method_name;
        return x_Qt_rbdisconnectrb(argc, argv, self);
    }
    // Qt5::rbconnect    
    else if (method_name == "rbconnect") {
        qDebug()<<"got ittttttt."<<method_name;
        return x_Qt_rbconnect(argc, argv, self);
    }
    // Qt5::connect
    else if (method_name == "connect") {
        qDebug()<<"got ittttttt."<<method_name;
        return x_Qt_connect(argc, argv, self);        
    }
    */
    
    assert(conn != NULL);
    return conn;
}

// for rb signal ==> rb slot
// signature1 Qt5::rbconnectrb(sobj, signal, robj, slot);
// signature2 Qt5::rbconnectrb(sobj, signal, Proc|Block|Lambda)
// TODO signature3 robj.rbconnectrb(sobj, signal, slot)
ConnectAny *ConnectFactory::create_rbconnectrb(int argc, VALUE *argv, VALUE obj)
{
    ConnectAny *conn = NULL;

    QString method_name = rb_id2name(SYM2ID(argv[0]));
    qDebug()<<method_name<<argc;

    qDebug()<<argc;
    // QString klass_name = get_rb_class(argv[1]);
    QString method_signature = RSTRING_PTR(argv[2]);
    QString signal_name = method_signature.split('(').at(0);

    if (argc < 3 || argc > 5) {
        qDebug()<<rb_block_given_p();
        qDebug()<<"parameter not match. need 3 to 5, given:"<<argc;
        return NULL;
    }

    // signature 2, block
    if (argc == 3 && rb_block_given_p()) {
        VALUE pblk = rb_block_proc(); // ok 
        VALUE lblk = rb_block_lambda(); // ok

        RubyConnectRuby *xconn = new RubyConnectRuby(argv[1], argv[2], lblk);
        QMetaObject::Connection qtconn =
            QObject::connect(xconn, SIGNAL(invoked(int, VALUE*, VALUE)), xconn, SLOT(route(int, VALUE*, VALUE)), Qt::QueuedConnection);
        xconn->qtconn = qtconn;
        xconn->installEventFilter(xconn);
        conn = xconn;
    } else if (argc == 3 && !rb_block_given_p()) {
        qDebug()<<"need a block here.";
        return NULL;
    }
    
    // signature 2, proc,lambda
    // qDebug()<<argc<<TYPE(argv[3])<<(TYPE(argv[3]) == T_DATA);
    // qDebug()<<RTYPEDDATA_P(argv[3])<<(RBASIC_CLASS(argv[3]) == rb_cProc);
    else if (argc == 4 && RBASIC_CLASS(argv[3]) == rb_cProc) {
        RubyConnectRuby *xconn = new RubyConnectRuby(argv[1], argv[2], argv[3]);
        QMetaObject::Connection qtconn =
            QObject::connect(xconn, SIGNAL(invoked(int, VALUE*, VALUE)), xconn, SLOT(route(int, VALUE*, VALUE)), Qt::QueuedConnection);
        xconn->qtconn = qtconn;
        xconn->installEventFilter(xconn);
        conn = xconn;
    } else if (argc == 4 && RBASIC_CLASS(argv[3]) != rb_cProc) {
        qDebug()<<"need a proc or lambda here.";
        return NULL;
    }

    // else, standard signature 1
    else {
        RubyConnectRuby *xconn = new RubyConnectRuby(argv[1], argv[2], argv[3], argv[4]);
        QMetaObject::Connection qtconn =
            QObject::connect(xconn, SIGNAL(invoked(int, VALUE*, VALUE)), xconn, SLOT(route(int, VALUE*, VALUE)), Qt::QueuedConnection);
        xconn->qtconn = qtconn;
        xconn->installEventFilter(xconn);
        conn = xconn;
    }

    return conn;
}
