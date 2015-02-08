#include "fix_clang_undef_eai.h"

#include <unistd.h>
#include <sys/syscall.h>

#include "debugoutput.h"

// simple 
// setenv("QT_MESSAGE_PATTERN", "[%{type}] %{appname} (%{file}:%{line}) T%{threadid} %{function} - %{message} ", 1);

// 关闭输出
// setevn QT_QUITE_DEBUG=1
// need -DQT_MESSAGELOGCONTEXT=1
// TODO 优化
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static char *qt_quite_debug = getenv("QT_QUITE_DEBUG");
    if (qt_quite_debug && qt_quite_debug[0] == '1') {
        return;
    }
    int tid = syscall(__NR_gettid);
    QDateTime now = QDateTime::currentDateTime();
    QString time_str = now.toString("yyyy-MM-dd hh:mm:ss"); // now.toString("yyyy-MM-dd hh:mm:ss.zzz");

    QStringList tlist = QString(context.file).split('/');
    QString hpath = tlist.takeAt(tlist.count() - 1);

    QString mfunc = QString(context.function);
    tlist = mfunc.split(' ');
    for (int i = tlist.size() - 1; i >= 0; i --) {
        if (tlist.at(i).indexOf('(') != -1) {
            tlist = tlist.at(i).split('(');
            break;
        }
    }
    /*
    if (tlist.at(0) == "static" || tlist.at(0) == "virtual") {
        
        if (tlist.size() >= 3) {
            tlist = tlist.takeAt(2).split('(');
        } else {
            fprintf(stderr, "ctx list size: %d\n", tlist.size());
            for (int i = 0 ; i < tlist.size(); i ++) {
                fprintf(stderr, "ctx: %d, %s\n", i, tlist.at(i).toLocal8Bit().data());
            }
        }
    } else {
        tlist = tlist.takeAt(1).split('(');
    }
    */
    mfunc = tlist.takeAt(0);
    
    // static void StunClient::debugStunResponse(QByteArray)
    // void StunClient::debugStunResponse(QByteArray)
    // virtual void StunClinet::aaa()
    // virtual QProcess::~QProcess()
    // C++11 unamed function: auto MultiWorker::ipv6Request(QString, QNetworkRequest)::<anonymous class>::operator()() const
    if (1) {
        fprintf(stderr, "[%s] T(%u) %s:%u %s - %s\n", time_str.toLocal8Bit().data(),  tid,
                hpath.toLocal8Bit().data(), context.line,
                mfunc.toLocal8Bit().data(), msg.toLocal8Bit().constData());
    } else {
        fprintf(stderr, "[%s] T(%u) %s:%u %s,%s - %s\n", time_str.toLocal8Bit().data(), tid,
                hpath.toLocal8Bit().data(), context.line,
                mfunc.toLocal8Bit().data(), context.function, msg.toLocal8Bit().constData());
    }
    return;

    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}
