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

void RubyConnectQt::call(int argc, const VALUE *argv)
{
    qDebug()<<argc<<"not impled";
}

void QtConnectQt::call(int argc, const VALUE *argv)
{
    qDebug()<<argc<<"not impled";
}

