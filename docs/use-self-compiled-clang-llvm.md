
# 自编译的clang/llvm，应该是个Release+Asserts版本，
# 要考虑是静态链接，默认方式应该全是静态链接的。
# mkdir build; cd build; ../configure --prefix=/home/dev/clang3.7

并且同时安装两个版本，3.5.x和3.7.x。

在普通电脑上编译llvm/clang耗时啊，
i3 CPU, 内存8G，
make -j3用时：86m9.829s
