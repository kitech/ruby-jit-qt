#ifndef VTYPE_H
#define VTYPE_H

#include <stdint.h>
#include <stdbool.h>

enum {
    Invalid = 0,
    BoolTy,
    IntTy,
    Int8Ty,
    Int16Ty,
    Int32Ty,
    Int64Ty,
    UintTy,
    Uint8Ty,
    Uint16Ty,
    Uint32Ty,
    Uint64Ty,
    UintptrTy,
    Float32Ty,
    Float64Ty,
    Complex64Ty,
    Complex128Ty,
    ArrayTy,
    ChanTy,
    FuncTy,
    InterfaceTy,
    MapTy,
    PtrTy,
    SliceTy,
    StringTy,
    StructTy,
    UnsafePointerTy,
};

#ifdef __cplusplus
#define _Bool bool
#endif

typedef struct _GoVar {
    int kind;
    uint32_t ukind;
    char *name;
    int size;
    void *goval; // from reflect.TypeOf()
    
    // union {
        _Bool b;
        int i32;
        uint32_t ui32;
        int64_t i64;
        uint64_t ui64;
        float  f32;
        char *str;
        void *star;
    // } v;
} GoVar;

typedef struct _GoVarArray {
    GoVar **vars;
    int n;
} GoVarArray;


#ifdef __cplusplus
extern "C" {
#endif

    GoVar *newGoVar();
    void freeGoVar(GoVar *var);

    GoVarArray *newGoVarArray(int n);
    void freeGoVarArray();
    void setElem(GoVarArray *vars, int n, GoVar *var);
    
#ifdef __cplusplus
};
#endif

#endif /* VTYPE_H */
