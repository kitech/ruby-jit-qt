### Clang AST 树遍历优化

目前要实现查找AST树中匹配方法或者函数symbol符号名字的Decl，通过遍历AST树方式实现。
但是，现在这个方式耗时很多，占用整个过程的50%时间，希望能在这个点上优化掉接近一半的时间。

以下就这个功能特点，分析整理可能的优化机制。

虽然所有是在整个AST树中，但实际不需要在整个树的根开始查找，而是从树的一个节点开始。
遍历过程，可能要加入新的新的结点。

实现上，可以看作一个森林数据结构，初始状态时，森林只有一个根节点。
执行过程中，遍历遇到另个函数声明，如果在这次调用中需要，则需要把这个节点加入到森林中，作为森林的一个新的根节点。

根据以上分析，实现执行会有回溯过程，并且回溯时的节点也变化了。
可以暂时把这种方式叫做回溯遍历方法。

考虑到这个过程，只需要关注inline的方法或者函数，可以不断进行深度遍历，
在整个遍历路径上遇符合条件的节点，回调执行相应的处理函数。
如果遍历方式没有问题的话，应该能够一次遍历完成所有的处理工作。
暂时把这种方式叫做单次扩展遍历方法。

分析完功能需求和可能的方式，接下来分析clang提供的遍历相关功能。

在clang/AST/目录下有几个xxxVisitor.h文件，用于遍历AST。
RecursiveASTVisitor.h
StmtVisitor.h
DeclVisitor.h
CommentVisitor.h
DataRecursiveASTVisitor.h
TypeLocVisitor.h
EvaluatedExprVisitor.h
TypeVisitor.h

第一个RecursiveASTVisitor.h提供的功能最多，正如其名字一样，实现的是递归遍历。
这也是可以使用的一个。
其余几项是针对某种类型进行遍历，并且不会递归遍历，只遍历一级节点。这几种不详细描述了。

RecursiveASTVisitor.h提供了一个基础模板类，声明如下，
template <typename Derived> class RecursiveASTVisitor {
}

RecursiveASTVisitor类提供了三种遍历方式，
1、递归遍历所有节点
2、按照类继承层次遍历，一直到最顶层基类。
3、按照用户覆盖的方法进行遍历。

这三种遍历方式分别对应三组方法：
1、TraverseDecl(Decl *x)执行第一种遍历。它是遍历以x为AST根的入口。
这个方法简单的分发到TraverseFoo(Foo *x)方法，这里Foo指的是\*x的动态类型。
该方法又会调用WalkUpFromFoo(x)，然后递归访问x的子节点。
TraverseStmt(Stmt *x), TraverseType(QualType x)以类型的方式执行。 
2、WalkUpFromFoo(Foo *x)，
它并不会试图遍历x的子节点。相反地，它会首先调用WalkUpFromBar(x)，
Bar是Foo的父类（除非Foo没有父类），然后调用VisitFoo(x)。
3、VisitFoo(Foo *x)实现第三种遍历。

并且，对于以上任何方法，如果返回true则继续递归遍历，如果返回false则中止遍历过程。

定义子类：
class FindSymbolVisitor : public clang::RecursiveASTVisitor<FindSymbolVisitor>
{}

源代码：
https://github.com/kitech/ruby-jit-qt/blob/master/ivm/findsymbolvisitor.cpp
https://github.com/kitech/ruby-jit-qt/blob/master/ivm/findsymbolvisitor.h


而之前是手写了一个递归树遍历算法，形成了森林状数据结构。
并且，之前的算法有可能多次重复调用，效率成倍降低。

目前最新代码是使用的基于RecursiveVisitor的单次递归遍历算法，一次遍历实现查找所有的symbol，
并记录symbol相应的Decl，返回后一次性生成所有的symbol的IR代码。
与之前的实现相比较，这种方式效率更高，更灵活通用且比较可靠，想要扩展也比较容易。

效率测试：
原来使用的算法的情况下的测试：
QDateTime("2015-03-09 16:13:15.280 CST Qt::LocalTime")
// 首次执行qt5.QString.new("abc")
QDateTime("2015-03-09 16:13:15.429 CST Qt::LocalTime") 49ms
QDateTime("2015-03-09 16:13:15.524 CST Qt::LocalTime")
// 二次执行qt5.QString.new("abc")
QDateTime("2015-03-09 16:13:15.657 CST Qt::LocalTime") 23ms

新算法的情况下的测试：
QDateTime("2015-03-09 16:14:56.295 CST Qt::LocalTime")
// 首次执行qt5.QString.new("abc")
QDateTime("2015-03-09 16:14:56.318 CST Qt::LocalTime") 23ms
QDateTime("2015-03-09 16:14:56.341 CST Qt::LocalTime")
// 二次执行qt5.QString.new("abc")
QDateTime("2015-03-09 16:14:56.348 CST Qt::LocalTime") 7ms

经过测试，这个优化，耗时只有原来的实现的40%。如果按最快情况下，耗时可能只有原来的30%。
新算法的情况下，首次执行仍旧使用了23ms，是因为C++模板实例化耗时还会用时15-16ms，
但C++模板实例化是一次性的，后续的使用不再重复执行，所以二次执行时只用了7ms。

根据以上测试数据，优化效果还是比较明显的。

