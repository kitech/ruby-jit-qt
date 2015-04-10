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

extern "C" void mymodule_dealloc( void *p )
{
    qDebug()<<"dealloccccccccing:"<<p;
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
    PyObject *pyoab = PyModule_Create(&abModule);
    PyObject *pyoqt = PyModule_Create(&qtModule);
    PyObject *pyoqt5 = PyModule_Create(&qt5Module);
    
    pyinit->m_cModuleQt = pyoqt5;
    return pyinit->m_cModuleQt;
    */
}

extern "C" void dbg_type(void *to)
{
    qDebug()<<to;
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

PyObject py_object_initializer =  {
    _PyObject_EXTRA_INIT
    1,
    NULL    // type must be init'ed by user
};

extern "C" void standard_dealloc( PyObject *p )
{
    PyMem_DEL( p );
}


extern "C" void extension_object_deallocator( PyObject *_self )
{
    qDebug()<<"dealloccccccccing:"<<_self;
    /*
    PythonClassInstance *self = reinterpret_cast< PythonClassInstance * >( _self );
#ifdef PYCXX_DEBUG
    std::cout << "extension_object_deallocator( self=0x" << std::hex << reinterpret_cast< unsigned int >( self ) << std::dec << " )" << std::endl;
    std::cout << "    self->m_pycxx_object=0x" << std::hex << reinterpret_cast< unsigned int >( self->m_pycxx_object ) << std::dec << std::endl;
#endif
    delete self->m_pycxx_object;
    _self->ob_type->tp_free( _self );
    */
}


void PyInit::initialize()
{
    qInstallMessageHandler(myMessageOutput);
    gce = new CtrlEngine();

    // Py_Initialize();
    
    static PyObject* cModuleQt = NULL;
    // abModule.m_free = mymodule_dealloc;
    cModuleQt = PyModule_Create(&abModule);
    // cModuleQt = PyModule_New("ab");
    m_cModuleQt = cModuleQt;

    int ok = false;
    ok = PyModule_Check(cModuleQt);

    qDebug()<<"mc.....:"<<ok<<PyModule_GetName(cModuleQt);
        //<<"'"<<PyModule_GetFilenameObject(cModuleQt)<<"'";
    qDebug()<<_Py_PackageContext;

    PyModule_AddStringConstant(cModuleQt, "hehe", "vvvvvvheheh");
    // PyType_Type();

    PyObject* dt = PyDict_New();
    // make_type("ab.QString456", (PyTypeObject*)&PyObject_Type, NULL, 0);
    // PyObject* dyc = PyObject_CallFunction((PyObject*)&PyType_Type, "sOO",
    // "QString456", &PyObject_Type, dt);
    // PyModule_AddObject(cModuleQt, "QString456", dyc);

    // see    ExtensionType.hxx:212  static PythonType &behaviors()
    //*
    auto tbl = new PyTypeObject();
    memset(tbl, 0, sizeof(PyTypeObject));
    *reinterpret_cast<PyObject*>(tbl) = py_object_initializer;
    reinterpret_cast<PyObject*>(tbl)->ob_type = &PyType_Type;
    tbl->tp_name = "ab.QString456";
    tbl->tp_basicsize = 8;
    tbl->tp_itemsize = 0;
    tbl->tp_flags |= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    // tbl->tp_dealloc = standard_dealloc;
    tbl->tp_dealloc = extension_object_deallocator;
    //*/
    qDebug()<<"ty ready???"<<PyType_Ready(tbl); // 这个函数的调用很重要啊，相当于类型定义结束。必须要执行的。
    
    PyModule_AddObject(cModuleQt, "QString456", (PyObject*)tbl);
    PyObject_Print((PyObject*)tbl, stdout, 1);
    PyObject_Print((PyObject*)cModuleQt, stdout, 1);

    int rc = PyRun_SimpleString("XX = type('QString456', (object,), dict())");
    qDebug()<<rc<<"ffffffff";
    
    // table->tp_name = const_cast<char *>( default_name );
	// table->tp_basicsize = basic_size;
	// table->tp_itemsize = itemsize;

    // p->set_tp_new( extension_object_new );
    // p->set_tp_init( extension_object_init );
    // p->set_tp_dealloc( extension_object_deallocator );

    qDebug()<<"Hhhhhhhhh";
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
