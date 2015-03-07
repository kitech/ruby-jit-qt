
#include "marshallruby.h"

#include "callargument.h"

const QVector<QVariant> &CallArgument::getArgs2()
{
    if (m_argc - m_offset < m_vargv2.count()) {
        m_vargv2 = MarshallRuby::ARGV2Variant(m_argc, m_argv, m_offset);
    }

    return m_vargv2;
}

const QVariant &CallArgument::getArg2(int idx)
{
    if (m_argc - m_offset < m_vargv.count()) {
        m_vargv2 = MarshallRuby::ARGV2Variant(m_argc, m_argv, m_offset);
    }

    return m_vargv2[idx];
}

const QVector<QSharedPointer<MetaTypeVariant> > &CallArgument::getArgs()
{
    qDebug()<<"callarg:"<<m_argc<<m_offset<<m_vargv.count();
    if (m_argc - m_offset >= m_vargv.count()) {
        m_vargv = MarshallRuby::ARGV2MTVariant(m_argc, m_argv, m_offset);
        qDebug()<<"callarg:"<<m_vargv;
    }

    return m_vargv;
}

const QSharedPointer<MetaTypeVariant> CallArgument::getArg(int idx)
{
    if (m_argc - m_offset < m_vargv.count()) {
        m_vargv = MarshallRuby::ARGV2MTVariant(m_argc, m_argv, m_offset);
    }

    return m_vargv[idx];
}


//////////////
MetaTypeVariant::MetaTypeVariant(int type, const void *paddr)
{
    mtype = type;    
    if (paddr) {
        maddr = QMetaType::create(QMetaType::QVariant, paddr);
    }
}

MetaTypeVariant::MetaTypeVariant(int type, QVariant& vaddr)
{
    mtype = type;
    maddr = QMetaType::create(QMetaType::QVariant, &vaddr);
}

QVariant MetaTypeVariant::toVariant() const
{
    QByteArray data;
    QDataStream stm(&data, QIODevice::WriteOnly);
    bool bret = QMetaType::save(stm, QMetaType::QVariant, maddr);
    QDataStream rdstm(&data, QIODevice::ReadOnly);
    QVariant rv;
    rdstm >> rv;
    return (rv);
}

