#ifndef CONNECTRUBY_H
#define CONNECTRUBY_H

#include <QtCore>

#include "ruby_cxx.h"

class ConnectAny : public QObject
{
    Q_OBJECT;
public:
    enum {
        CONN_QT_TO_QT = 0,
        CONN_QT_TO_RUBY = 1,
        CONN_RUBY_TO_RUBY = 2,
        CONN_RUBY_TO_QT = 3,
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
    virtual void call(int argc, const VALUE *argv);
    
    QObject *m_sender;
    QString m_signal;

    VALUE m_receiver;
    VALUE m_slot;
    ID m_slot_id;
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

#endif /* CONNECTRUBY_H */
