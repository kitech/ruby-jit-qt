#include <cassert>

#include <private/qobject_p.h>

#include "qtobjectmanager.h"
#include "marshallruby.h"
#include "connectruby.h"


void ConnectAny::call_ruby(int argc, const VALUE *argv)
{
    if (m_conn_type == CONN_RUBY_TO_RUBY) {
        RubyConnectRuby *r2r = (RubyConnectRuby*)this;
        VALUE rbret = rb_funcall3(r2r->m_receiver, r2r->m_slot, argc, argv);
    }
    else if (m_conn_type == CONN_QT_TO_RUBY) {
        QtConnectRuby *q2r = (QtConnectRuby*)this;
        VALUE rbret = rb_funcall3(q2r->m_receiver, q2r->m_slot, argc, argv);
    }
}

void ConnectAny::call_qt(int argc, const VALUE *argv)
{
    
}

void RubyConnectRuby::call(int argc, const VALUE *argv)
{
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
    assert(this->mcevt != NULL);
}

void RubyConnectQt::call(int argc, const VALUE *argv)
{
    qDebug()<<argc<<"not impled";
}

void QtConnectQt::call(int argc, const VALUE *argv)
{
    qDebug()<<argc<<"not impled";
}

////
// static
ConnectAny *ConnectFactory::create(int argc, VALUE *argv, VALUE obj)
{
    ConnectAny *conn = NULL;

    QString method_name = rb_id2name(SYM2ID(argv[0]));
    qDebug()<<method_name;

    if (method_name == "qtconnectrb") {
        QObject *sender = (QObject*)Qom::inst()->getObject(argv[1]);
        QVariant vsignal = MarshallRuby::VALUE2Variant(argv[2]);
        QString signal = vsignal.toString();
        QString rsignal = QString("2%1").arg(signal); // real signal

        QtConnectRuby *xconn = new QtConnectRuby(sender, rsignal, argv[3]);
        QMetaObject::Connection qtconn =
            QObject::connect(sender, rsignal.toLatin1().data(), xconn, SLOT(router()), Qt::QueuedConnection);
        xconn->installEventFilter(xconn); // ahaha, niubeee

        xconn->qtconn = qtconn;
        conn = xconn;
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

