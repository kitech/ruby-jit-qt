#ifndef UTILS_H
#define UTILS_H

#include <ruby.h>

#include "qom.h"

#define SAVE_CI0(value) Qom::inst()->objs[rb_hash(self)] = (QObject*)value
#define SAVE_CI2(type, value) Qom::inst()->objs[rb_hash(self)] = (QObject*)value
// ci == cpp instance 
 #define GET_CI2(type) type *ci = (type*)Qom::inst()->objs[rb_hash(self)]

#define GET_CI3(type) y##type *ci = (y##type*)Qom::inst()->objs[rb_hash(self)]
#define GET_CI0() QObject *ci = (QObject*)Qom::inst()->objs[rb_hash(self)]

#define FUNVAL (VALUE (*) (...))

// from src/corelib/kernel/qobjectdefs.h
// # define METHOD(a) "0"#a
// # define SLOT(a) "1"#a
// # define SIGNAL(a) "2"#a
// # define emit   // empty

// TODO use
template<typename QtObjectType>
void save_ci(VALUE self) {
    QtObjectType *io = new QtObjectType();
    Qom::inst()->objs[rb_hash(self)] = (QObject*)io;
    QMetaObject *mo = io->metaObject();
}

#endif /* UTILS_H */










