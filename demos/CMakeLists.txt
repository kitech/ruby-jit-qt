
add_executable(tp1  var_tpl.cpp)
qt5_use_modules(tp1 Core)

add_executable(rtip  rtip.cpp)
target_link_libraries(rtip ${CLANGLIBS})
qt5_use_modules(rtip Core)

add_executable(ast_match ast_match.cpp)
target_link_libraries(ast_match ${CLANGLIBS})
qt5_use_modules(ast_match Core)

add_executable(qtparser qtparser.cpp)
target_link_libraries(qtparser ${CLANGLIBS})
qt5_use_modules(qtparser Core)

add_executable(jitrun jitrun.cpp)
target_link_libraries(jitrun ${CLANGLIBS})
qt5_use_modules(jitrun Core)
# set_target_properties(jitrun PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "clang_pch_src.h")
# set_target_properties(jitrun PROPERTIES COTIRE_UNITY_LINK_LIBRARIES_INIT "COPY")
# cotire(jitrun)
# 好像和llvm中的一个库不兼容。CommandLine.h，可能是因为这行用了个...Args，这是C++14的特性，PCH搞不定。

add_executable(genir gen_ir_code.cpp)
target_link_libraries(genir ${CLANGLIBS})
qt5_use_modules(genir Core)
