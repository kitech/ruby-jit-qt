#ifndef _DEBUGOUTPUT_H_
#define _DEBUGOUTPUT_H_

#include <QtCore>


// usage:
// #include "debugoutput.h"
// qInstallMessageHandler(myMessageOutput);

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);


#endif /* _DEBUGOUTPUT_H_ */
