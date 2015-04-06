
在Qt中有两个重要的工具，rcc和uic。
rcc是实现资源文件嵌入到最终程序的一种方式，它生成C++表示的资源文件原始数据，并提供相应的api实现资源读取。
uic是把.ui文件生成一个C++源文件的方式，简化gui部分界面的生成工作。

在做不同语言的qt绑定时，这两个工具是需要考虑的。

在ruby语言绑定中，已经提供了一个，不过这个算是从qtruby中移植过来的。
现在要把这个移植步骤分析整理一下，为go语言甚至其他语言绑定做准备，以便更完整的语言绑定功能。

uic/utils.h 需要添加一个toxxlangIdentifier函数，转变成xxlang格式的变量符号。
添加QT\_UIC\_xxlang_GENERATOR编译宏。通过宏编译条件，在uic/uic.h和uic/uic.cpp中实现相应的xxlangwrite(DomUI *ui)方法。
在uic/ui4.{h,cpp}文件中实现一些常量字符串的转换。





