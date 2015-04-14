
### 动态注册一个module

### 动态注册一个类
可以动态注册一个qt模块。也可以动态注册一个qt类到该qt模块。
但是还无法实现在遇到AttributeError时再即时注册一个qt类，
因为截获了AttributeError后，即使注册了新的不存在的Qt类，程序也会退出。
这是python的方式：打印出错误后即退出程序。


AttributeError调用栈输出：
(gdb) bt
#0  0x00007ffff71424b7 in raise () from /usr/lib/libc.so.6
#1  0x00007ffff714388a in abort () from /usr/lib/libc.so.6
#2  0x00007ffff713b41d in __assert_fail_base () from /usr/lib/libc.so.6
#3  0x00007ffff713b4d2 in __assert_fail () from /usr/lib/libc.so.6
#4  0x00000000004c4b00 in PyObject_GetAttr (v=0x7ffff674e5d8, name=0x7ffff66a0658) at Objects/object.c:890
#5  0x00000000005a2c09 in PyEval_EvalFrameEx (f=0xa35a68, throwflag=0) at Python/ceval.c:2420
#6  0x00000000005aadb1 in PyEval_EvalCodeEx (_co=0x7ffff7e74d00, globals=0x7ffff7f13bb8, locals=0x7ffff7f13bb8, args=0x0, 
    argcount=0, kws=0x0, kwcount=0, defs=0x0, defcount=0, kwdefs=0x0, closure=0x0) at Python/ceval.c:3588
#7  0x0000000000596c44 in PyEval_EvalCode (co=0x7ffff7e74d00, globals=0x7ffff7f13bb8, locals=0x7ffff7f13bb8)
    at Python/ceval.c:775
#8  0x0000000000423d07 in run_mod (mod=0xa6fdf8, filename=0x7ffff66a27b0, globals=0x7ffff7f13bb8, locals=0x7ffff7f13bb8, 
    flags=0x7fffffffa570, arena=0xa34930) at Python/pythonrun.c:2180
#9  0x0000000000423a63 in PyRun_FileExFlags (fp=0xa6fa60, filename_str=0x7ffff7f17130 "../main2.py", start=257, 
    globals=0x7ffff7f13bb8, locals=0x7ffff7f13bb8, closeit=1, flags=0x7fffffffa570) at Python/pythonrun.c:2133
#10 0x0000000000421a13 in PyRun_SimpleFileExFlags (fp=0xa6fa60, filename=0x7ffff7f17130 "../main2.py", closeit=1, 
    flags=0x7fffffffa570) at Python/pythonrun.c:1606
#11 0x00000000004208a0 in PyRun_AnyFileExFlags (fp=0xa6fa60, filename=0x7ffff7f17130 "../main2.py", closeit=1, 
    flags=0x7fffffffa570) at Python/pythonrun.c:1292
#12 0x000000000043a32c in run_file (fp=0xa6fa60, filename=0x9a2120 L"../main2.py", p_cf=0x7fffffffa570)
    at Modules/main.c:319
#13 0x000000000043af65 in Py_Main (argc=4, argv=0x97b020) at Modules/main.c:751
#14 0x000000000041adff in main (argc=4, argv=0x7fffffffa768) at ./Modules/python.c:69


(gdb) bt
#0  0x00007ffff71424b7 in raise () from /usr/lib/libc.so.6
#1  0x00007ffff714388a in abort () from /usr/lib/libc.so.6
#2  0x00007ffff713b41d in __assert_fail_base () from /usr/lib/libc.so.6
#3  0x00007ffff713b4d2 in __assert_fail () from /usr/lib/libc.so.6
#4  0x00000000005c8310 in PyErr_Format (exception=0x90bec0 <_PyExc_AttributeError>, 
    format=0x6786c0 "'%.50s' object has no attribute %s:%d '%U'") at Python/errors.c:806
#5  0x00000000004c559a in _PyObject_GenericGetAttrWithDict (obj=0x7ffff674e5d8, name=0x7ffff66a0658, dict=0x7ffff66a2448)
    at Objects/object.c:1111
#6  0x00000000004c56af in PyObject_GenericGetAttr (obj=0x7ffff674e5d8, name=0x7ffff66a0658) at Objects/object.c:1124
#7  0x00000000004c4aad in PyObject_GetAttr (v=0x7ffff674e5d8, name=0x7ffff66a0658) at Objects/object.c:884
#8  0x00000000005a2bf0 in PyEval_EvalFrameEx (f=0xa35a68, throwflag=0) at Python/ceval.c:2420
#9  0x00000000005aad98 in PyEval_EvalCodeEx (_co=0x7ffff7e74d00, globals=0x7ffff7f13bb8, locals=0x7ffff7f13bb8, args=0x0, 
    argcount=0, kws=0x0, kwcount=0, defs=0x0, defcount=0, kwdefs=0x0, closure=0x0) at Python/ceval.c:3588
#10 0x0000000000596c2b in PyEval_EvalCode (co=0x7ffff7e74d00, globals=0x7ffff7f13bb8, locals=0x7ffff7f13bb8)
    at Python/ceval.c:775
#11 0x0000000000423d07 in run_mod (mod=0xa0cd88, filename=0x7ffff66a27b0, globals=0x7ffff7f13bb8, locals=0x7ffff7f13bb8, 
    flags=0x7fffffffa570, arena=0xa666b0) at Python/pythonrun.c:2180
#12 0x0000000000423a63 in PyRun_FileExFlags (fp=0xa0c9f0, filename_str=0x7ffff7f17130 "../main2.py", start=257, 
    globals=0x7ffff7f13bb8, locals=0x7ffff7f13bb8, closeit=1, flags=0x7fffffffa570) at Python/pythonrun.c:2133
#13 0x0000000000421a13 in PyRun_SimpleFileExFlags (fp=0xa0c9f0, filename=0x7ffff7f17130 "../main2.py", closeit=1, 
    flags=0x7fffffffa570) at Python/pythonrun.c:1606
#14 0x00000000004208a0 in PyRun_AnyFileExFlags (fp=0xa0c9f0, filename=0x7ffff7f17130 "../main2.py", closeit=1, 
    flags=0x7fffffffa570) at Python/pythonrun.c:1292
#15 0x000000000043a32c in run_file (fp=0xa0c9f0, filename=0x9a2120 L"../main2.py", p_cf=0x7fffffffa570)
    at Modules/main.c:319
#16 0x000000000043af65 in Py_Main (argc=4, argv=0x97b020) at Modules/main.c:751
#17 0x000000000041adff in main (argc=4, argv=0x7fffffffa768) at ./Modules/python.c:69

经过这个跟踪，已经定位到hook点了，替换module的tp_getattro函数，
可截获请求python类的事件，并实现动态的Python类定义。
并且，这种方式不需要再截获sys.excepthook事件了，因为在这截获已经迟了。

动态创建类型还是一个大难题，总是做不到像type('Abc', (object,), dict())的这种效果，
因为一直出现AttributeError: 'module' object has no attribute 'Abc'的错误。
还是要考虑找到type函数的实现，参考它实现创建一个有效的类类型的实现函数。
或者也可以动态调用这个内置的type函数。
创建一个自定义类型，see file:///usr/share/doc/python/html/extending/newtypes.html?highlight=type
已经解决这个即使返回正确的PyTypeObject依旧报错的问题，要手动清除错误标识：PyErr_Clear();

### 动态注册一个方法
在实现了类的动态截获与注册之后，还要截获类的方法调用。
主要是使用PyMethod\_New 和 PyCFunction\_NewEx两个函数。

值得注意的是，python在真正调用那个注册进去的C函数的时候，不会把方法信息带着，
而只是带了实例信息和参数信息，这儿目前使用了一种非常hack的方式实现把方法信息带到被调用的C函数中。


### C API的内存计数与回收问题
到底是回收呢还是计数呢？
怎么设置是回收还是不回收呢？

### 使用方式
##### import qt5;
##### 创建类实例
python语法api用法示例: s = qt5.QString()

这相当于C++的new QString()的作用。

##### 调用实例方法
python语法api用法示例: s.length()

这相当于C++的s->length()的作用。

##### 调用静态方法
python语法api用法示例: ret = qt5.QString.someStaticMethod()

这相当于C++的QString::someStaticMethod();

##### 调用qt的全局函数
python语法api用法示例: ret = qt5.qrand()

这相当于C++的ret = qrand();

##### 调用qt的全局常量
python语法api用法示例:
ret = qt5.Qt.Window
ret = qt5.Qt.SubWindow

##### 调用qt类的常量
python语法api用法示例:
ret = qt5.QWidget.AlignLeft
ret = qt5.QWidget.AlignRight


### 遇到的一个问题
qrand()函数每次调用的结果全部相同，这是为什么呢？
哦，原来也需要qsrand()函数做seed啊。我记得以前都不需要手动执行qsrand()函数的。不过问题不大。

