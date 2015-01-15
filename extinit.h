#ifndef EXTINIT_H
#define EXTINIT_H

#include <QtCore>

class ExtInit
{
public:
    ExtInit() {}
    virtual ~ExtInit(){}

public:
    virtual void initialize() = 0;
};

#endif /* EXTINIT_H */
