#include "fix_clang_undef_eai.h"

#include <QtCore>

#include "metar_classes_qtcore.h"
#include "metar_classes_qtgui.h"
#include "metar_classes_qtwidgets.h"
#include "metar_classes_qtnetwork.h"

#include "metas.h"

// all ok
/*
QHash<QString, QString> __rq_protos = {{"abc", "123"}};
QMap<QString, QString> __bb = {{"abc", "123"}};
std::map<std::string, std::string> __aa = {{"abc", "123"}};

char *__rq_protos_[] = {
    "abc", "123",
};
*/
QHash<QString, QString> __rq_protos = {
    // format: {"key", "value"},
    #include "metar_protos.cpp"
};
QHash<QString, const QMetaObject *> __rq_metas;

static bool inited = false;
void init_class_metas()
{
    if (inited) return;
    inited = true;

    // format: __rq_metas["yQObject"] = &yQObject::staticMetaObject;
    #include "metar_objects.cpp"
}





















