; ModuleID = 'forlli.cpp'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-pc-linux-gnu"

@.str = private unnamed_addr constant [19 x i8] c"run from aaamain:\0A\00", align 1
@_ZL1s = internal global i8* getelementptr inbounds ([16 x i8]* @.str3, i32 0, i32 0), align 4
@.str1 = private unnamed_addr constant [16 x i8] c"run from main:\0A\00", align 1
@.str2 = private unnamed_addr constant [23 x i8] c"run from nomanglefun:\0A\00", align 1
@.str3 = private unnamed_addr constant [16 x i8] c"e hello ll IR.\0A\00", align 1

define void @_Z5amainv() #0 {
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([19 x i8]* @.str, i32 0, i32 0))
  ret void
}

declare i32 @printf(i8*, ...) #0

define void @_Z6forlliv() #0 {
  %1 = load i8** @_ZL1s, align 4
  %2 = call i32 (i8*, ...)* @printf(i8* %1)
  ret void
}

define i32 @main() #0 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1
  %2 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([16 x i8]* @.str1, i32 0, i32 0))
  ret i32 21
}

define i32 @nomanglefun() #0 {
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([23 x i8]* @.str2, i32 0, i32 0))
  ret i32 34
}

attributes #0 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5.0 (tags/RELEASE_350/final)"}
