
#include "debugoutput.h"
#include "ctrlengine.h"

#include "go/api/src/qt/dynamic/vtype.h"
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

extern "C" void *gx_Qt_class_method_missing(int argc, void *argv, void *obj)
{ return goinit->Qt_class_method_missing(argc, argv, obj); }

void GoInit::initialize()
{
    qInstallMessageHandler(myMessageOutput);
    gce = new CtrlEngine();
    qDebug()<<"here";
}

#define EARGS(argv, klass) \
    GoVarArray *vars = (GoVarArray*)argv; \
    QString cls = QString((const char*)klass);

void *GoInit::Qt_class_new(int argc, void *argv, void *klass)
{
    EARGS(argv, klass);
    qDebug()<<argc<<vars->n<<cls;

    // return 0;
    QVector<QVariant> uargs;

    if (cls == "QApplication") {
        uargs.append(QVariant(0));
        QStringList sl;
        // sl << "main";
        uargs.append(sl);
    }
    
    void *o = gce->vm_new(cls, uargs);
    qDebug()<<o;
    return o;
    return 0;
}

void *GoInit::Qt_class_method_missing(int argc, void *argv, void *obj)
{
    GoVarArray *vars = (GoVarArray*)argv;
    qDebug()<<argc<<vars;

    GoVar *var;

    var = vars->vars[0];
    QString klass_name = QString((char*)(var->str));
    qDebug()<<klass_name;
    
    var = vars->vars[1];
    QString method_name = QString((char*)(var->str));
    qDebug()<<(char*)(var->str)<<method_name;

    QVector<QVariant> uargs;
    QVariant rv = gce->vm_call(obj, klass_name, method_name, uargs);
    
    return 0;
}
