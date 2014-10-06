
#include <stdlib.h>
#include <stdio.h>

/*
  生成.ll IR 汇编语言
  clang++ -S  -x c++ -emit-llvm forlli.cpp

  解释执行：
  lli  -entry-function=_Z6forlliv forlli.ll
 */
void amain()
{
    printf("run from aaamain:\n");   
}

static const char *s = "e hello ll IR.\n";
void forlli()
{
    printf(s);
}
int main()
{
    printf("run from main:\n");
    return 21;
}

extern "C" int nomanglefun() {
    printf("run from nomanglefun:\n");
    return 34;
}






