* 手工编写Qt5的ruby语言绑定







* 自动化绑定翻译的问题，
参数，函数的prototype原型问题，导致翻译代码不通用。

使用Qt的meta object机制，为每个Qt类生成所有方法的meta信息。
这样就可以使用名字字符串调用任意的Qt方法了。
不过，对于像QTcpSocket这类，大多数方法全部是继续过来的，那么就不能直接使用了。(很严重的问题）
   这种就需要同时继续两个类，一个Qxxx类，一个yQxxx类。甚至更多基类。
   是不是在解析的时候可以考虑处理一下。但这样yQxxx子类中的用于查找的slot方法将非常多。
   不过这种方案应该可行，可以考虑，复杂度越来越高了。

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

JIT的另一个选择？libjit from gnu.(other google c++ jit compiler)

机器需求，至少双核CPU，要使用-arch core2

* C++11在语言绑定中的应用



* LLVM 代码分析与匹配


* LLVM IR代码生成，与lli,llc,llvm-link工具
 clvm c/c++ language virtual marchine


* 用于Qt5内部的QArrayData类分析。



