

#include "qom.h"

Qom *Qom::_inst = NULL;
Qom::Qom()
{
}

Qom *Qom::inst()
{
    if (Qom::_inst == NULL) {
        Qom::_inst = new Qom;
        // init_fmap();
    }
    return Qom::_inst;
}

void Qom::testParser()
{
    // get_class_method("abcdef.h");
}

void Qom::testIR()
{
    // run_code_jit();
}








