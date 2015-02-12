##### name resolution for void* => class object

##### Q:
如:
[code type="ruby"]
label = Qt5::QLabel.new('alabel')
sp = label.sizePolicy()
label.setSizePolicy(sp)     # name resolution error
[/code]

##### A:
而clang++编译该代码的C++版的输出如下，其实也是无法实现名称决议的。
a.cpp:25:12: error: no matching member function for call to 'setSizePolicy'
    label->setSizePolicy(&sp);
    ~~~~~~~^~~~~~~~~~~~~
/usr/include/qt/QtWidgets/qwidget.h:501:10: note: candidate function not viable: no known conversion from 'QSizePolicy *' to
      'QSizePolicy' for 1st argument; remove &
    void setSizePolicy(QSizePolicy);
         ^
/usr/include/qt/QtWidgets/qwidget.h:844:22: note: candidate function not viable: requires 2 arguments, but 1 was provided
inline void QWidget::setSizePolicy(QSizePolicy::Policy hor, QSizePolicy::Policy ver)
                     ^
1 error generated.

因此，
需要对这种情况下给出的void*类型参数自动做预处理，预处理成可实现名字决议的参数形式。
在C++中，需要做的预处理为，
QSizePolicy sp2(*(QSizePolicy*)((void*)&sp)); // compile ok
label->setSizePolicy(sp2);
也就是需要该类有构造函数，可以通过现有的指针解引用后创建一个新的临时对象。

也许现在只传递一个void*类型的参数信息量不够用，应该需要更丰富的类型信息才能做更好的解释处理。

放行之后，生成的代码，不过这还有个问题，是把void*的地址当作值传递给方法调用了，应该传递的是void*解引后的值。
declare void @_ZN7QWidget13setSizePolicyE11QSizePolicy(%class.QWidget*, i32) #0
define void* @jit_main() {
eee:
  call void @_ZN7QWidget13setSizePolicyE11QSizePolicy(void** inttoptr (i64 19231088 to void**), i32 inttoptr (i64 20414352 to i32))
  ret void
}

==================================================
[gzleo@oarchbox1 wangpanfs]$ ruby main2.rb 2>&1| grep "replaced:"
[2015-02-12 16:00:11] T(20287) ctrlengine.cpp:213 i32ToRecord - i32ToRecord replaced: 0x32bac50
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: setSizePolicy
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:11] T(20287) ctrlengine.cpp:213 i32ToRecord - i32ToRecord replaced: 0x37958b0
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: setSizePolicy
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addLayout
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addLayout
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addLayout
[2015-02-12 16:00:11] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:12] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:12] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:12] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addItem
[2015-02-12 16:00:12] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:12] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:12] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addLayout
[2015-02-12 16:00:12] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:12] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addWidget
[2015-02-12 16:00:12] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: addLayout
[2015-02-12 16:00:12] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: setCentralWidget
[2015-02-12 16:00:12] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: setMenuBar
[2015-02-12 16:00:12] T(20287) frontengine.cpp:1327 FrontEngine::replace_trivial_copy_params - replaced: setStatusBar

最终解决方式，把参数替换成相应的i32类型的值，修改uargs。
这种参数类型标识为，是
if (ptype->isObjectType() && ptype.isTriviallyCopyableType(mrgunit->getASTContext())
    && !ptype->isPointerType() && !ptype->isReferenceType()) {
}


##### 有些返回类类型的方法，生成的IR代码显示返回i32，无法正确处理返回值 #2

###### Q:
如，QWidget::sizePolicy()方法，生成的IR代码如下，
declare i32 @_ZNK7QWidget10sizePolicyEv(%class.QWidget*) #0

define void* @jit_main() {
eee:
%0 = call i32 @_ZNK7QWidget10sizePolicyEv(void** inttoptr (i64 18559616 to void**))
ret i32 %0
}

如果按照返回一个类对象的方式生成，应该是一个sret，可能的结果是，
declare i32 @_ZNK7QWidget10sizePolicyEv(%class.QSizePolicy , %class.QWidget) #0

define void* @jit_main() {
eee:
%0 = call i32 @_ZNK7QWidget10sizePolicyEv(%class.QSizePolicy , void* inttoptr (i64 18559616 to void**))
ret i32 %0
}

###### A:




 
