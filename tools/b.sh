set -x


CC=/usr/bin/clang CXX=/usr/bin/clang++ \
  /mnt/sda4/mptemp/llvm-3.5.0.src/./configure \
  --prefix=/myllvm \
  --enable-shared \
  --enable-libffi \
  --enable-targets="host,x86,x86_64,cpp" \
  --disable-expensive-checks \
  --enable-debug-runtime \
  --enable-keep-symbols \
  --enable-assertions \
  --with-optimize-option="-O0 -Os" \
  --disable-clang-static-analyzer \
  --disable-clang-arcmt \
  --enable-bindings=none \
  --disable-docs \
  --without-python

#   --enable-debug-symbols \ 或者 -g 参数导致程序或者库文件太大，4G内存的电脑基本很难处理
# 调整--enable-assertions和 -O0两个参数吧
# make REQUIRES_RTTI=1
# rm -vf Release+Asserts/bin/clang Release+Asserts/lib/libclang.so Release+Asserts/lib/libclang.a Release+Asserts/lib/libclangCodeGen.a
