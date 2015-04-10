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

// #define PyMODINIT_FUNC extern "C" PyObject*
extern "C" PyObject* PyInit_qt5()
{
    Init_forpy();
    return pyinit->m_cModuleQt;
}

// 定义 python 的 model
static struct PyModuleDef abModule = { PyModuleDef_HEAD_INIT, "ab", NULL, -1, NULL};
static struct PyModuleDef qtModule = { PyModuleDef_HEAD_INIT, "qt", NULL, -1, NULL};
static struct PyModuleDef qt5Module = { PyModuleDef_HEAD_INIT, "qt5", NULL, -1, NULL};

#define MODDEF(pmname) \
    static struct PyModuleDef pmname##Module = { PyModuleDef_HEAD_INIT, ""#pmname, NULL, -1, NULL };

PyObject py_object_initializer =  {
    _PyObject_EXTRA_INIT
    1,
    NULL    // type must be init'ed by user
};

#include <frameobject.h>
_Py_IDENTIFIER(excepthook);
_Py_IDENTIFIER(extract_tb);

extern "C" PyObject* qgc_excepthook(PyObject* self, PyObject* args)
{
    qDebug()<<"catched sys.excepthook call:"<<self<<args;
    assert(self == pyinit->m_cModuleQt);
    
    PyObject *exc, *value, *tb;
    if (!PyArg_UnpackTuple(args, "excepthook", 3, 3, &exc, &value, &tb))
        return NULL;
    
    qDebug()<<exc<<value<<tb;
    PyObject_Print(exc, stdout, 1); puts("\n");
    PyObject_Print(value, stdout, 1); puts("\n");
    PyObject_Print(tb, stdout, 1); puts("\n");
    PyTracebackObject *tbo = (PyTracebackObject*)tb;
    PyFrameObject *fro = tbo->tb_frame;
    PyCodeObject *coo = fro->f_code;
    PyObject_Print(coo->co_code, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(coo->co_consts, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(coo->co_name, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(coo->co_names, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(coo->co_varnames, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(coo->co_cellvars, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(coo->co_filename, stdout, 1); puts("aaaaaaaaaa\n");

    PyImport_ImportModuleNoBlock("traceback");
    PyObject* tbm = PyImport_ImportModuleNoBlock("traceback");
    _object* rl = _PyObject_CallMethodId(tbm, &PyId_extract_tb, "O", tb);
    PyObject_Print(rl, stdout, 1); puts("aaaaaaaaaa\n");
    qDebug()<<PyList_Size(rl);
    PyObject* rt = PyList_GetItem(rl, 0); // a tuple object
    PyObject_Print(rt, stdout, 1); puts("aaaaaaaaaa\n");
    qDebug()<<PyTuple_Size(rt);
    PyObject* rs = PyTuple_GetItem(rt, 3);
    PyObject_Print(rs, stdout, 1); puts("aaaaaaaaaa\n");
    qDebug()<<PyUnicode_AsUTF8(rs);
    QString eline = PyUnicode_AsUTF8(rs);

    // TODO 现在只是定位到一行，还需要更进一步准确定位。
    // 例如，a = eg.QString123()
    // AttributeError: 'module' object has no attribute 'QString123'
    // 对于AttributeError的情况，匹配出来attribute名字，在这一行中回溯到非空格位置，
    // 然后再用以下检测方法检测。
    if (eline.startsWith("qt5.Q")) {
        QString klass = eline.split(".").at(1);
        
        return Py_None;        
    } else if (eline.startsWith("qt5.q")) {
        QString fname = eline.split(".").at(1);
        
        return Py_None;
    } else if (eline.startsWith("qt5.")) {
        QString cname = eline.split(".").at(1);
        
        return Py_None;
    } else {
        // not care, goon
    }
    
    PyErr_Display(exc, value, tb);
    Py_INCREF(Py_None);
    return Py_None;
}

//_Py_static_string(PyId_excepthook, excepthook);
void PyInit::initialize()
{
    qInstallMessageHandler(myMessageOutput);
    gce = new CtrlEngine();
    Py_Initialize();

    // 注册qt5模块，即qt5.py
    PyObject* cModuleQt = NULL;
    cModuleQt = PyModule_Create(&qt5Module);
    m_cModuleQt = cModuleQt;

    int ok = false;
    ok = PyModule_Check(cModuleQt);

    qDebug()<<"mc.....:"<<ok<<PyModule_GetName(cModuleQt)
        // 这一行导致一个报错：SystemError: initialization of qt5 raised unreported exception
        // 但是如果有注册的类之后，这个却不会有什么问题。
            <<"'"<<PyModule_GetFilenameObject(cModuleQt)<<"'";
    qDebug()<<_Py_PackageContext;

    // for little testing.
    this->registClass("QString456");
    this->registClass("QString789");

    // 注册类方法不存在事件处理
    // 这里需要一个模块级的sys.excepthook
    auto hook = _PySys_GetObjectId(&PyId_excepthook);
    PyObject_Print((PyObject*)hook, stdout, 1);
    puts("\n");

    _PySys_SetObjectId(&PyId_excepthook, NULL);
    static PyMethodDef exh = {"pqc_excepthook", qgc_excepthook, METH_VARARGS, "qgc_excepthook"};
    PyObject* exh_fn = PyCFunction_NewEx(&exh, cModuleQt, NULL);
    PyObject_Print((PyObject*)exh_fn, stdout, 1);
    puts("\n");
    qDebug()<<METH_VARARGS<<PyCFunction_GET_FLAGS(exh_fn)
            <<(PyCFunction_GET_FLAGS(exh_fn) & ~(METH_CLASS | METH_STATIC | METH_COEXIST));
    _PySys_SetObjectId(&PyId_excepthook, exh_fn);

    hook = _PySys_GetObjectId(&PyId_excepthook);
    PyObject_Print((PyObject*)hook, stdout, 1);
    puts("\n");
    
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

//Py_Finalize();

bool PyInit::registClass(QString klass)
{
    // see    ExtensionType.hxx:212  static PythonType &behaviors()
    //*
    auto NewTypeObject = [](QString klass) -> auto /* PyTypeObject* */ {
        auto tbl = new PyTypeObject();
        memset(tbl, 0, sizeof(PyTypeObject));
        *reinterpret_cast<PyObject*>(tbl) = py_object_initializer;
        reinterpret_cast<PyObject*>(tbl)->ob_type = &PyType_Type;
        tbl->tp_name = "qt5.QString456";
        tbl->tp_name = strdup(QString("qt5.%1").arg(klass).toLatin1().data());
        tbl->tp_basicsize = 8;
        tbl->tp_itemsize = 0;
        tbl->tp_flags |= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
        // tbl->tp_dealloc = standard_dealloc;
        // tbl->tp_dealloc = extension_object_deallocator;
        //*/

        // table->tp_name = const_cast<char *>( default_name );
        // table->tp_basicsize = basic_size;
        // table->tp_itemsize = itemsize;

        // p->set_tp_new( extension_object_new );
        // p->set_tp_init( extension_object_init );
        // p->set_tp_dealloc( extension_object_deallocator );
        
        qDebug()<<"ty ready???"<<PyType_Ready(tbl); // 这个函数的调用很重要啊，相当于类型定义结束。必须要执行的。

        return tbl;
    };
    auto tbl = NewTypeObject(klass);
    
    // PyModule_AddObject(cModuleQt, "QString456", (PyObject*)tbl);
    PyModule_AddObject(m_cModuleQt, klass.toLatin1().data(), (PyObject*)tbl);
    PyObject_Print((PyObject*)tbl, stdout, 1);
    puts("\n");
    PyObject_Print((PyObject*)m_cModuleQt, stdout, 1);
    puts("\n");
    
    this->m_classes[klass] = tbl;

    return true;
}

////// tryingggggggggg codessssssssssss
extern "C" void mymodule_dealloc( void *p )
{
    qDebug()<<"dealloccccccccing:"<<p;
}

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

void PyInit_initialize()
{
    qInstallMessageHandler(myMessageOutput);
    // gce = new CtrlEngine();

    Py_Initialize();
    
    static PyObject* cModuleQt = NULL;
    // abModule.m_free = mymodule_dealloc;
    cModuleQt = PyModule_Create(&abModule);
    // cModuleQt = PyModule_New("ab");
    // m_cModuleQt = cModuleQt;

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
