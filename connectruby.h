#ifndef CONNECTRUBY_H
#define CONNECTRUBY_H

#include <QtCore>

#include "ruby_cxx.h"

class ConnectAny : public QObject
{
    Q_OBJECT;
public:
    enum {
        CONN_QT_TO_QT = 1,
        CONN_QT_TO_RUBY = 2,
        CONN_RUBY_TO_RUBY = 3,
        CONN_RUBY_TO_QT = 4,
    };

public:
    inline int type() { return m_conn_type;}
    virtual void call(int argc, const VALUE *argv) = 0;

protected:
    void call_ruby(int argc, const VALUE *argv);
    void call_qt(int argc, const VALUE *argv);
    
protected:
    int m_conn_type = 0;
    QString m_signal_name;
    QString m_slot_name;
};

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
        m_slot = slot;
    }

    virtual void call(int argc, const VALUE *argv);    
    
    VALUE m_sender;
    VALUE m_signal;

    VALUE m_receiver;
    VALUE m_slot;
    ID m_slot_id;
};

class QMetaCallEvent;
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
        m_slot = slot;
    }

    QtConnectRuby(QObject *sender, QString signal, VALUE slot)
        : QtConnectRuby() {
        m_sender = sender;
        m_signal = signal;
        
        m_slot = slot;
    }
    
    virtual void call(int argc, const VALUE *argv);
    
    QObject *m_sender;
    QString m_signal;

    VALUE m_receiver;
    VALUE m_slot;
    ID m_slot_id;
    
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
    virtual void call(int argc, const VALUE *argv);
    
    VALUE m_sender;
    VALUE m_signal;

    QObject *m_receiver;
    QString m_slot;
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
    virtual void call(int argc, const VALUE *argv);
    
    QObject *m_sender;
    QString m_signal;
    
    QObject *m_receiver;
    QString m_slot;

    //
    QMetaObject::Connection m_qt_conn;
};

/*
  根据参数，识别是哪种connect，并且创建适当的xxConnectxx实例
 */
class ConnectFactory
{
public:
    static ConnectAny *create(int argc, VALUE *argv, VALUE obj);
};

#endif /* CONNECTRUBY_H */
