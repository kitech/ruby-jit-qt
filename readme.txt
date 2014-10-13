* 手工编写Qt5的ruby语言绑定







* 自动化绑定翻译的问题，
参数，函数的prototype原型问题，导致翻译代码不通用。

使用Qt的meta object机制，为每个Qt类生成所有方法的meta信息。
这样就可以使用名字字符串调用任意的Qt方法了。
不过，对于像QTcpSocket这类，大多数方法全部是继续过来的，那么就不能直接使用了。(很严重的问题）
   这种就需要同时继续两个类，一个Qxxx类，一个yQxxx类。甚至更多基类。
   是不是在解析的时候可以考虑处理一下。但这样yQxxx子类中的用于查找的slot方法将非常多。
   不过这种方案应该可行，可以考虑，复杂度越来越高了。

在使用moc的时候，碰到一个问题，有时候执行过程中指针变成了Qt基类的指针，无法确定是不是绑定的子类对象。

目录结构：
metalize/{core,gui,widgets,network}
metalize/metas_auto.cpp
moc/{core,gui,widgets,network}

排除的类：
qabstractxxx类

排除的方法：
operatorxxx方法


* 动态绑定中的问题与JIT，
不显式生成翻译使用的代码，像smoke和swig和rice等。
也许可以有一个C/C++系统虚拟机CVM(像JVM)
生成PCH文件，clang++ -cc1 -x c++ -emit-pch clang_pch_src.h


组件，
entry(ruby <==> c++)
translator( c++ <==> qtcode)
clvm ( qtcode <==> native)
resolve_symbol
resolve_return_type
astbuilder,用于函数原型的核对和默认值相关的处理。便是有一个ast在内存中会点内存非常大。

JIT的另一个选择？libjit from gnu.(other google c++ jit compiler)

机器需求，至少双核CPU，要使用-arch core2。这个不需要，当时没有加载相关的类库。

基本理清了纯JIT实现的思路，但是，在没有编译器的情况下，Qt的inline函数无法在符号表中找到。
所以代码也就无法执行，inline的代码全部在头文件中，
所以这种实现方式，还需要解析头文件，把代码编译成执行代码。
这样比较麻烦啊，可能还需要处理一遍头文件中的inline方法。
可以通过一次继续实现导出所有的方法符号，工程量也比较大，但是比较使用Qt moc方式导出要好些。
另外在使用jit的情况下，还需要注意方法重载的问题。

默认情况下，链接程序只处理程序中使用过了的符号，没有使用的符号不会出现在最终ELF的符号表中。

使用把cpp和h文件分开的方法，能够保留符号表。

符号表导出到文本中，当作一个明文文本的数据结构，编译进程序，实现符号表的查找。

方法的原型，生成明文文本的数据结构，编译进程序，实现方法的原型查找和结果类型的查找。

不过这现在搞的程序即使strip -s之后也有18M，太大了，不知道是什么原因导致程序这么大。

不是以前两个文件数据的问题，即使不编译加载进程序，程序也会这么大。

是因为使用的libLLVM*.a和libclang*.a的原因吗？

好像libLLVM的代码还好些，可能使用libclang*的程序文件大小问题更严重。

在使用IR的call指令调用直接返回对象拷贝的方法时，需要特殊处理。
可能是调用桟的顺序有关。是返回的处理，StructRet属性（sret)。

类的inline方法，只在编译时才转换成代码，并不存在于.so中。
使用Yaxxx继承封装，解决了一部分问题，但也带来了另一部分问题，
就是类层次结构与原来不一样了，动态查找时需要特殊处理的地方太多了。
可以考虑预先把inline函数转换成.ll代码，或者更实时地把inline方法转换成.ll代码。
这种使用方法，需要依赖llvm的源代码中的头文件，这些头文件并不是llvm/clang API的一部分。


现在的基本框架已经试验完成，还需要解决的实用问题，
更多的功能测试。
线程安全。
jit虚拟机的优化。
生成的IR代码的优化。
更通用化。动态加载外部引用库，转接到其他语言。
动态读取symbol符号表。
动态reflict实现方法原型读取。

* C++11在语言绑定中的应用



* LLVM 代码分析与匹配


* LLVM IR代码生成，与lli,llc,llvm-link工具
 clvm c/c++ language virtual marchine


* 用于Qt5内部的QArrayData类分析。


* 关于C++的一点整理
原来函数的默认值是在编译时在调用端处理的。
编译器如果发现传递的参数不够，并且函数有默认参数值，则分配相应的临时变量，
再执行函数调用，也就是在函数体的实现中不需要做任何的处理。

C++的函数名方法名的mangle，是一种简化的压缩格式。

C++符号表中的类型，T,U,D,b,t,W,u,V,B,d,R,r

C++的方法或者函数，如果返回是结构体或者类，不是引用或者指针或者基本数据类型(int, char, long, void*)，
则编译器会在调用前分配结构体的内存空间，通过第一个参数传递给被调用函数。
而在编译这类被调用函数时，编译器同样把第一个参数的位置定义为返回值的位置，
其他的参数依次类推向后移动。
在llvm中，如何判断方法或者函数是这类型的，才能生成对应的IR。

形参和实参，传进去之前的形参，在函数内部用到的参数叫实参，因为它有值。


资料：
http://woboq.com/blog/reflection-in-cpp-and-qt-moc.html
http://woboq.com/blog/moc-with-clang.html
http://llvm.lyngvig.org/Articles/Mapping-High-Level-Constructs-to-LLVM-IR


