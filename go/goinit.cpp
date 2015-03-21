
#include "debugoutput.h"
#include "ctrlengine.h"

#include "goinit.h"

static GoInit *goinit = NULL;

extern "C" void Init_forgo()
{
    GoInit *init = new GoInit();
    init->initialize();
    goinit = init;
}

void GoInit::initialize()
{
    qInstallMessageHandler(myMessageOutput);
    gce = new CtrlEngine();
}


