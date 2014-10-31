#ifndef QTHDRSRC_H
#define QTHDRSRC_H

/*
  用于生成qthdrsrc.ast文件
  clang++ -x c++ -S -emit-ast "./qthdrsrc.h" -fPIC -I/usr/include/qt -I/usr/include/qt/QtCore \
      -I/usr/include/qt/QtGui -I/usr/include/qt/QtWidgets -I/usr/include/qt/QtNetwork
 */


#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtNetwork>

#include "metalize/metar_classes_qtcore.h"
#include "metalize/metar_classes_qtgui.h"
#include "metalize/metar_classes_qtwidgets.h"
#include "metalize/metar_classes_qtnetwork.h"


#endif /* QTHDRSRC_H */


