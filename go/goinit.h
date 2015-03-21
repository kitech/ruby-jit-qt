#ifndef GOINIT_H
#define GOINIT_H

#include "extinit.h"

class GoInit : public ExtInit
{
public:
    GoInit() : ExtInit()
    {
    }

    virtual void initialize();

protected:
    class CtrlEngine *gce = NULL;
    
public:
    
public: // register
    
protected:
    
};


#endif /* GOINIT_H */
