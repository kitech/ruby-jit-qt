
A Qt5 binding for Ruby language. Using clang/llvm JIT engine.

###Why new Qt5 binding for Ruby
Though it Qt 5.4 now, qtbinding not still support Qt5.  
And so the origin kdebindings-qtruby and kdebindings-smokeqt projects.  


###Features
Qt5+  
Bidirector Singal/Slot between Ruby and Qt  
Easy keep update to version by version  

###Examples
######widget and object
    require 'Qt5'
    a = Qt5::QApplication.new(ARGV.count, ARGV)
    w = Qt5::QPushButton.new("it's a push button")
    w.show
    a.exec

#####core class
    require 'Qt5'
    s = Qt5::QString.new
    s.append("abcdefg")
    puts 'len=' + s.length.to_s    


###Install
    git clone git@github.com:kitech/ruby-jit-qt.git
    cd ruby-jit-qt
    cmake .
    make

###Supported Modules
    QtCore
    QtGui
    QtWidgets
    QtNetwork
    ... more later

###TechStack
    clang/llvm/IR
    Ruby C API
    Qt5
    

