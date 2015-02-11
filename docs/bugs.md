

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



 
