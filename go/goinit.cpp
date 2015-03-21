
#include "debugoutput.h"
#include "ctrlengine.h"

#include "go/api/src/qgc/vtype.h"
#include "goinit.h"

static GoInit *goinit = NULL;
extern "C" void Init_forgo()
{
    GoInit *init = new GoInit();
    init->initialize();
    goinit = init;
}

extern "C" void *gx_Qt_class_new(int argc,  void *argv, void *self)
{ return goinit->Qt_class_new(argc, argv, self); }


void GoInit::initialize()
{
    qInstallMessageHandler(myMessageOutput);
    gce = new CtrlEngine();
}

#define EARGS(argv, klass) \
    GoVarArray *vars = (GoVarArray*)argv; \
    QString cls = QString((const char*)klass);

void *GoInit::Qt_class_new(int argc, void *argv, void *klass)
{
    EARGS(argv, klass);
    qDebug()<<vars->n<<cls;

    // return 0;
    QVector<QVariant> uargs;

    if (cls == "QApplication") {
        uargs.append(QVariant(1));
        QStringList sl;
        sl << "progname";
        uargs.append(sl);
    }
    
    void *o = gce->vm_new(cls, uargs);
    qDebug()<<o;
    return o;
    return 0;
}

