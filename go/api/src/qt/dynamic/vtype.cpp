#include <cstring>
#include <memory>

#include "vtype.h"

GoVar *newGoVar()
{
    GoVar *var = (GoVar*)calloc(1, sizeof(GoVar));
    memset((void*)var, 0, sizeof(var));
    return var;
    return 0;
}

void freeGoVar(GoVar *var)
{
    free(var->name);
    free(var);
}

GoVarArray *newGoVarArray(int n)
{
    GoVarArray *vars = (GoVarArray*)calloc(1, sizeof(GoVarArray));
    vars->vars = (GoVar**)calloc(n, sizeof(GoVar*));
    
    return vars;
}

void freeGoVarArray()
{
}

void setElem(GoVarArray *vars, int n, GoVar *var)
{
    vars->vars[n] = var;
}
   

