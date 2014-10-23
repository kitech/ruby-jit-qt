// 用于生成llvm jit可使用的类型
// 最好是在编译时动态生成，否则不同系统结果不一样，如x86_64和x86_32生成的不太一样。
// usage:
// clang++ -S -emit-llvm metalize/jit_types.cpp -I. -I/usr/include/qt/QtCore/ -I/usr/include/qt -fPIC -std=c++11 -I/usr/include/qt/QtGui -I/usr/include/qt/QtWidgets -I/usr/include/qt/QtNetwork

#include "metalize/metar_classes_qtcore.h"
#include "metalize/metar_classes_qtgui.h"
#include "metalize/metar_classes_qtwidgets.h"
#include "metalize/metar_classes_qtnetwork.h"

void __keep_jit_types()
{
    void *v0 = NULL;
    // eg.
    // (YaQString*)v0;
    // (YaQUrl*)v0;
    #include "jit_types_body.cpp"
}

/*
  由于目前发现一些方法无法正常动态生成ll代码
  使用这种方式可以预先生成这部分代码，在动态调用这部分代码。
  后续动态生成ll代码优化之后，这部分则可去掉。
 */
void __keep_jit_cannot_gen_functions()
{
    QTypedArrayData<unsigned short>::sharedNull();
    QTypedArrayData<char>::sharedNull();
    // LLVM ERROR: Program used external function '_ZNK10QByteArray4sizeEv' which could not be resolved!
    
    {

    }
}


















