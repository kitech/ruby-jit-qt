#ifndef KITECH_QTCORE_H
#define KITECH_QTCORE_H

#include <QtCore>

#include <ruby.h>

// namespace kitech {
/*
class yQObject : public QObject
{
    Q_OBJECT;
public:
    Q_INVOKABLE yQObject() {}

public slots:

    // Q_INVOKABLE QMetaObject::Connection 	connect(const QObject * sender, const char * signal, const QObject * receiver, const char * method, Qt::ConnectionType type = Qt::AutoConnection);
    // Q_INVOKABLE QMetaObject::Connection 	connect(const QObject * sender, const QMetaMethod & signal, const QObject * receiver, const QMetaMethod & method, Qt::ConnectionType type = Qt::AutoConnection);
    // Q_INVOKABLE QMetaObject::Connection 	connect(const QObject * sender, PointerToMemberFunction signal, const QObject * receiver, PointerToMemberFunction method, Qt::ConnectionType type = Qt::AutoConnection);
    // Q_INVOKABLE QMetaObject::Connection 	connect(const QObject * sender, PointerToMemberFunction signal, Functor functor);
    // Q_INVOKABLE QMetaObject::Connection 	connect(const QObject * sender, PointerToMemberFunction signal, const QObject * context, Functor functor, Qt::ConnectionType type = Qt::AutoConnection);
    // Q_INVOKABLE bool 	disconnect(const QObject * sender, const char * signal, const QObject * receiver, const char * method);
    // Q_INVOKABLE bool 	disconnect(const QObject * sender, const QMetaMethod & signal, const QObject * receiver, const QMetaMethod & method);
    // Q_INVOKABLE bool 	disconnect(const QMetaObject::Connection & connection);
    // Q_INVOKABLE bool 	disconnect(const QObject * sender, PointerToMemberFunction signal, const QObject * receiver, PointerToMemberFunction method);
    // Q_INVOKABLE QString 	tr(const char * sourceText, const char * disambiguation = 0, int n = -1);

    void hehah()
    {}

    void on_emu_signal()
    {
        qDebug()<<this<<"goted 0 arg";
    }

    void on_emu_signal(int a)
    {
        qDebug()<<this<<"goted"<<a;
    }

signals:
    void hhhhhh();
    void emu_signal();
    void emu_signal(int);
};

*/
    // 由于QObject不支持拷贝构造函数，所以这种方式限制很大。
/*
    class yQString : public QObject, public QString
    {
        Q_OBJECT;

    public:
        yQString() : QObject(), QString(){}
        virtual ~yQString() {}

    public slots:
        QString & append(const QString & str) { return ::QString::append(str); }
		// QString & append(const QStringRef & reference) { return ::QString::append(reference);}
		// QString & append(const QChar * str, int len) {return ::QString::append(str, len);}
		// QString & append(QLatin1String str) { retrun ::QString::append(str); }
		QString & append(const QByteArray & ba) { return ::QString::append(ba); }
		QString & append(const char * str) { return ::QString::append(str); }
		QString & append(QChar ch) { return ::QString::append(ch); }

        QString arg(const QString & a, int fieldWidth = 0, QChar fillChar = QLatin1Char( ' ' )) const
        {return ::QString::arg(a, fieldWidth, fillChar); }
        QString arg(const QString & a1, const QString & a2) const
        { return ::QString::arg(a1, a2); }       
        QString arg(const QString & a1, const QString & a2, const QString & a3) const
        { return ::QString::arg(a1, a2, a3); }


        // yQString arg(int a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char( ' ' )) const
		// { yQString s; s.yi = this->yi; return s;}       
		// yQString arg(uint a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char( ' ' )) const
		// {}        
		// yQString arg(long a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char( ' ' )) const		 
        // {}

        void 	chop(int n) { yi.chop(n); }
        void 	clear() { yi.clear(); }
        int 	length() const { return ::QString::length();}

    private:
        QString yi; // y's instance
    };
*/
// };

extern "C" {
    int register_qtcore_methods(VALUE module);
};

#endif

