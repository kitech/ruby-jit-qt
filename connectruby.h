#ifndef CONNECTRUBY_H
#define CONNECTRUBY_H

#include <cassert>
#include <QtCore>

#include "ruby_cxx.h"

class ConnectAny : public QObject
{
    Q_OBJECT;
public:
    enum {
        CONN_NONE = 0,
        CONN_QT_TO_QT = 1,
        CONN_QT_TO_RUBY = 2,
        CONN_RUBY_TO_RUBY = 3,
        CONN_RUBY_TO_QT = 4,
    };

public:
    inline int type() { return m_conn_type;}
    virtual void call(int argc, VALUE *argv) = 0;
    // 是否两个conn的slot相等
    virtual bool slotEqual(ConnectAny *other) {
        assert(1==2); return false;
    }
    // 异步调用这个conn的slot
    virtual void acall(int argc, VALUE *argv, VALUE obj)
    { assert(1==2); }
    virtual QString toString() { assert(1==2); return QString(); }

protected:
    void call_ruby(int argc, const VALUE *argv);
    void call_qt(int argc, const VALUE *argv);
    
protected:
    int m_conn_type = 0;
    QString m_signal_name;
    QString m_slot_name;
};

class QMetaCallEvent;
/*
  使用Qt的QueuedConnection机制，把所有的metacall转换成QEvent::metaCall
  这样会减慢信号速度，但最好是能很好地解决rb<=>qt间的信号互通问题。
 */
class RubyConnectRuby : public ConnectAny
{
    Q_OBJECT;
public:
    RubyConnectRuby() {m_conn_type = CONN_RUBY_TO_RUBY;}
    RubyConnectRuby(VALUE sender, VALUE signal, VALUE receiver, VALUE slot)
        : RubyConnectRuby() {
        m_sender = sender;
        m_signal = signal;
        
        m_receiver = receiver;
        m_slot_id = SYM2ID(slot);
        m_slot = slot;

        // 等价的，把这个变量设置为不能被gc的
        // rb_global_variable(&m_receiver);
        rb_gc_register_address(&m_receiver); 
    }

    RubyConnectRuby(VALUE sender, VALUE signal, VALUE receiver)
        : RubyConnectRuby() {
        m_sender = sender;
        m_signal = signal;
        
        m_receiver = receiver;
        m_slot_id = rb_intern("call");
        m_slot = ID2SYM(m_slot_id);

        // 等价的，把这个变量设置为不能被gc的
        // rb_global_variable(&m_receiver);
        rb_gc_register_address(&m_receiver); 
    }
    ~RubyConnectRuby()
    {
        rb_gc_unregister_address(&m_receiver);
    }
    
    virtual void call(int argc, VALUE *argv);
    virtual bool slotEqual(ConnectAny *other) {
        if (other->type() != type()) return false;
        auto conn = (RubyConnectRuby*)other;
        return conn->m_receiver == m_receiver && conn->m_slot == m_slot;
    }
    
    virtual void acall(int argc, VALUE *argv, VALUE obj)
    {
        emit invoked(argc, argv, obj);
    }
    virtual QString toString()
    { return QString("sender(%1)::signal(%2) => receiver(%3)::slot(%4)")
            .arg(m_sender).arg(m_signal).arg(m_receiver).arg(m_slot); }    
    
    VALUE m_sender = 0;
    VALUE m_signal = 0;

    VALUE m_receiver = 0;
    VALUE m_slot = 0;
    ID m_slot_id = 0;

    QMetaObject::Connection qtconn;
    QMetaCallEvent *mcevt = NULL;
public:
    virtual bool eventFilter(QObject * watched, QEvent * event);
public slots:
    void router(int argc, VALUE *argv, VALUE obj);  
signals:
    void invoked(int argc, VALUE *argv, VALUE obj);
};

class QtConnectRuby : public ConnectAny
{
    Q_OBJECT;
public:
    QtConnectRuby() {m_conn_type = CONN_QT_TO_RUBY;}
    QtConnectRuby(QObject *sender, QString signal, VALUE receiver, VALUE slot)
        : QtConnectRuby() {
        m_sender = sender;
        m_signal = signal;
        
        m_receiver = receiver;
        m_slot_id = SYM2ID(slot);
        m_slot = slot;

        // 等价的，把这个变量设置为不能被gc的
        // rb_global_variable(&m_receiver);
        rb_gc_register_address(&m_receiver);
    }

    QtConnectRuby(QObject *sender, QString signal, VALUE lambda)
        : QtConnectRuby() {
        m_sender = sender;
        m_signal = signal;

        m_receiver = lambda; // blk,lamba,proc
        m_slot_id = rb_intern("call");
        m_slot = ID2SYM(m_slot_id);

        // 等价的，把这个变量设置为不能被gc的
        // rb_global_variable(&m_receiver);
        rb_gc_register_address(&m_receiver); 
    }
    ~QtConnectRuby()
    {
        rb_gc_unregister_address(&m_receiver);
    }
    
    virtual void call(int argc, VALUE *argv);
    virtual bool slotEqual(ConnectAny *other) {
        if (other->type() != type()) return false;
        auto conn = (QtConnectRuby*)other;
        return conn->m_receiver == m_receiver && conn->m_slot == m_slot;
    }
    
    virtual QString toString()
    { return QString("sender(%1)::signal(%2) => receiver(%3)::slot(%4)")
            .arg((qint64)m_sender).arg(m_signal).arg(m_receiver).arg(m_slot); }    
    
    
    QObject *m_sender = NULL;
    QString m_signal;

    VALUE m_receiver = 0;
    VALUE m_slot = 0;
    ID m_slot_id = 0;
    
    QMetaObject::Connection qtconn;
    QMetaCallEvent *mcevt = NULL;
public:
    virtual bool eventFilter(QObject * watched, QEvent * event);
public slots:
    void router();
};

class RubyConnectQt : public ConnectAny
{
    Q_OBJECT;
public:
    RubyConnectQt() {m_conn_type = CONN_RUBY_TO_QT;}
    RubyConnectQt(VALUE sender, VALUE signal, QObject *receiver, QString slot)
        : RubyConnectQt() {
        m_sender = sender;
        m_signal = signal;
        
        m_receiver = receiver;
        m_slot = slot;
    }
    virtual void call(int argc, VALUE *argv);
    virtual bool slotEqual(ConnectAny *other) {
        if (other->type() != type()) return false;
        auto conn = (RubyConnectQt*)other;
        return conn->m_receiver == m_receiver && conn->m_slot == m_slot;
    }
    
    virtual void acall(int argc, VALUE *argv, VALUE obj)
    {
        emit invoked(argc, argv, obj);
    }
    virtual QString toString()
    { return QString("sender(%1)::signal(%2) => receiver(%3)::slot(%4)")
            .arg(m_sender).arg(m_signal).arg((qint64)m_receiver).arg(m_slot); }    
    
    VALUE m_sender = 0;
    VALUE m_signal = 0;

    QObject *m_receiver = NULL;
    QString m_slot;
    
    QMetaObject::Connection qtconn;
    QMetaCallEvent *mcevt = NULL;
public:
    virtual bool eventFilter(QObject * watched, QEvent * event);
public slots:
    void router(int argc, VALUE *argv, VALUE obj);  
signals:
    void invoked(int argc, VALUE *argv, VALUE obj);
    
};

class QtConnectQt : public ConnectAny
{
    Q_OBJECT;
public:
    QtConnectQt() {m_conn_type = CONN_QT_TO_QT;}
    QtConnectQt(QObject *sender, QString signal, QObject *receiver, QString slot)
        : QtConnectQt() {
        m_sender = sender;
        m_signal = signal;

        m_receiver = receiver;
        m_slot = slot;
    }
    virtual void call(int argc, VALUE *argv);
    virtual bool slotEqual(ConnectAny *other) {
        if (other->type() != type()) return false;
        auto conn = (QtConnectQt*)other;
        return conn->m_receiver == m_receiver && conn->m_slot == m_slot;
    }
    
    virtual QString toString()
    { return QString("sender(%1)::signal(%2) => receiver(%3)::slot(%4)")
            .arg((qint64)m_sender).arg(m_signal).arg((qint64)m_receiver).arg(m_slot); }
    
    QObject *m_sender = NULL;
    QString m_signal;
    
    QObject *m_receiver = NULL;
    QString m_slot;

    //
    QMetaObject::Connection m_qt_conn;
    QMetaObject::Connection qtconn;
    QMetaCallEvent *mcevt = NULL;
public:
    virtual bool eventFilter(QObject * watched, QEvent * event);
public slots:
    void router();
};

/*
  根据参数，识别是哪种connect，并且创建适当的xxConnectxx实例
 */
class ConnectFactory
{
public:
    static ConnectAny *create(int argc, VALUE *argv, VALUE obj);
    static ConnectAny *create_rbconnectrb(int argc, VALUE *argv, VALUE obj);
    static ConnectAny *create_rbconnectqt(int argc, VALUE *argv, VALUE obj);
};

#endif /* CONNECTRUBY_H */
