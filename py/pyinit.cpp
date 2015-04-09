#include <Python.h>
// 设置 UNICODE 库，这样的话才可以正确复制宽字符集
#define UNICODE


#include "debugoutput.h"

#include "ctrlengine.h"
#include "callargument.h"

#include "utils.h"


#include "pyinit.h"


// 扩展入口函数Init_extname
static PyInit *pyinit = NULL;

extern "C" void Init_forpy()
{
    PyInit *init = new PyInit();
    init->initialize();
    pyinit = init;
}

// 定义 python 的 model
static struct PyModuleDef abModule = { PyModuleDef_HEAD_INIT, "ab", NULL, -1, NULL};
static struct PyModuleDef qtModule = { PyModuleDef_HEAD_INIT, "qt", NULL, -1, NULL};
static struct PyModuleDef qt5Module = { PyModuleDef_HEAD_INIT, "qt5", NULL, -1, NULL};

#define MODDEF(pmname) \
    static struct PyModuleDef pmname##Module = { PyModuleDef_HEAD_INIT, ""#pmname, NULL, -1, NULL };

// #define PyMODINIT_FUNC extern "C" PyObject*
extern "C" PyObject* PyInit_ab()
{
    Init_forpy();
    return pyinit->m_cModuleQt;
    MODDEF(abcdefg);
    // MODDEF(qt);
    // MODDEF(qt5);
    /*
    PyObject *pyoab = PyModule_Create(&aMmodule);
    PyObject *pyoqt = PyModule_Create(&qtModule);
    PyObject *pyoqt5 = PyModule_Create(&qt5Module);
    
    pyinit->m_cModuleQt = pyoqt5;
    return pyinit->m_cModuleQt;
    */
}

////////////////

static PyTypeObject* make_type(char *type, PyTypeObject* base, char**fields, int num_fields)
{
    PyObject *fnames, *result;
    int i;
    fnames = PyTuple_New(num_fields);
    if (!fnames) return NULL;
    for (i = 0; i < num_fields; i++) {
        PyObject *field = PyUnicode_FromString(fields[i]);
        if (!field) {
            Py_DECREF(fnames);
            return NULL;
        }
        PyTuple_SET_ITEM(fnames, i, field);
    }
    result = PyObject_CallFunction((PyObject*)&PyType_Type, "s(O){sOss}",
                    type, base, "_fields", fnames, "__module__", "_ast");
    Py_DECREF(fnames);
    return (PyTypeObject*)result;
}

void PyInit::initialize()
{
    qInstallMessageHandler(myMessageOutput);
    gce = new CtrlEngine();

    Py_Initialize();
    static PyObject* cModuleQt = NULL;
    cModuleQt = PyModule_Create(&abModule);
    m_cModuleQt = cModuleQt;

    PyModule_AddStringConstant(cModuleQt, "hehe", "vvvvvvheheh");
    // PyType_Type();

    PyObject* dt = PyDict_New();
    // make_type("ab.QString456", (PyTypeObject*)&PyObject_Type, NULL, 0);
    PyObject* dyc = PyObject_CallFunction((PyObject*)&PyType_Type, "sOO",
                  "QString456", &PyObject_Type, dt);
    // PyModule_AddObject(cModuleQt, "QString456", dyc);
    
    /*
    // 对所有的Qt5::someconst常量的调用注册
    rb_define_module_function(cModuleQt, "const_missing", FUNVAL nx_Qt_constant_missing, -1);
    // 对所有的Qt5::somefunc()函数的调用注册
    rb_define_module_function(cModuleQt, "method_missing", FUNVAL nx_Qt_method_missing, -1);
  
    // for qApp 类Qt全局变量
    rb_define_virtual_variable("$qApp", (VALUE (*)(ANYARGS)) nx_Qt_global_variable_get,
                               (void (*)(ANYARGS)) nx_Qt_global_variable_set);
    */
}
