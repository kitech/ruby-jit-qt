
#include "marshallruby.h"

#include "callargument.h"

const QVector<QVariant> &CallArgument::getArgs()
{
    if (m_argc - m_offset < m_vargv.count()) {
        m_vargv = MarshallRuby::ARGV2Variant(m_argc, m_argv, m_offset);
    }

    return m_vargv;
}

const QVariant &CallArgument::getArg(int idx)
{
    if (m_argc - m_offset < m_vargv.count()) {
        m_vargv = MarshallRuby::ARGV2Variant(m_argc, m_argv, m_offset);
    }

    return m_vargv[idx];
}

