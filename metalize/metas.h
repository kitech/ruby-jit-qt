#ifndef RQ_METAS_H
#define RQ_METAS_H

#include <QHash>
#include <QString>
#include <QMetaObject>

extern QHash<QString, QString> __rq_protos;
extern QHash<QString, const QMetaObject *> __rq_metas;
void init_class_metas();

#endif /* RQ_METAS_H */
