project(handby)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_VERBOSE_MAKEFILE on)

# add_definitions(-g -O2 -std=c++11) # -std=c++14 -fno-rtti)  # -fno-rtti
add_definitions(-DQT_MESSAGELOGCONTEXT=1 -DLLVM_DUMP_COLOR=1)
# set(CMAKE_C_COMPILER ./myllvm/bin/clang)
# set(CMAKE_CXX_COMPILER ./myllvm/bin/clang++)
# set(CMAKE_C_COMPILER clang)
# set(CMAKE_CXX_COMPILER clang++)

# 优先考虑环境变量，如果得到的值不是clang，则改成默认的clang
if(NOT "$ENV{CC}" STREQUAL "")
  set(CMAKE_C_COMPILER $ENV{CC})
endif(NOT "$ENV{CC}" STREQUAL "")
if(NOT "$ENV{CXX}" STREQUAL "")
  set(CMAKE_CXX_COMPILER $ENV{CXX})
endif(NOT "$ENV{CXX}" STREQUAL "")

if(NOT "${CMAKE_C_COMPILER}" MATCHES ".*clang*")
  set(CMAKE_C_COMPILER clang)
endif(NOT "${CMAKE_C_COMPILER}" MATCHES ".*clang*")
if(NOT "${CMAKE_CXX_COMPILER}" MATCHES ".*clang\\+\\+*")
  set(CMAKE_CXX_COMPILER clang++)
endif(NOT "${CMAKE_CXX_COMPILER}" MATCHES ".*clang\\+\\+*")

set(CMAKE_CXX_FLAGS "-g -O2 -std=c++11 -fno-exceptions -fno-cxx-exceptions") # -std=c++14")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-rpath,. -Wl,--export-dynamic -fno-inline")
#####这个设置虽然在有重复定义的情况下编译成功了，但会导致llvm的一个错误。
####CommandLine Error: Option 'load' registered more than once!
# set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--allow-multiple-definition")

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
include(cotire)
set(COTIRE_MINIMUM_NUMBER_OF_TARGET_SOURCES 1)

set(CMAKE_INCLUDE_CURRENT_DIR on)
set(CMAKE_AUTOMOC on)
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Network)
# find_package(Qt5WebSockets)

find_package(Ruby)
message("ruby?? ${RUBY_FOUND}, ${RUBY_INCLUDE_DIRS}, ${RUBY_LIBRARY}")
include_directories(${RUBY_INCLUDE_DIRS} ${Qt5Core_PRIVATE_INCLUDE_DIRS})

set(LLVM_CONFIG $ENV{LLVM_CONFIG})
if("${LLVM_CONFIG}" STREQUAL "")
    set(LLVM_CONFIG llvm-config)
endif("${LLVM_CONFIG}" STREQUAL "")
exec_program(${LLVM_CONFIG} ARGS --cxxflags 
  OUTPUT_VARIABLE LLVM_CXXFLAGS_OUTPUT RETURN_VALUE LLVM_CXXFLAGS_VALUE)
message("clang defs: ${LLVM_CXXFLAGS_OUTPUT}")
# message(${LLVM_CXXFLAGS_VALUE})

exec_program(${LLVM_CONFIG} ARGS --libs
  OUTPUT_VARIABLE LLVM_LIBS_OUTPUT RETURN_VALUE LLVM_LIBS_VALUE)
# message("llvm libs: ${LLVM_LIBS_OUTPUT}")

#add_definitions(${LLVM_CXXFLAGS_OUTPUT} )
#set(COMPILER_FLAGS ${LLVM_CXXFLAGS_OUTPUT})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXXFLAGS_OUTPUT}")

### fix c++11 => c++14 => c++17
string(REPLACE "-std=gnu++11" "-std=c++14" NEW_FLAGS ${CMAKE_CXX_FLAGS})
string(REPLACE "-std=c++11" "-std=c++14" NEW_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${NEW_FLAGS}")

# set(LLVM_SRC_DIR2 "./llvm-3.5.0.src")  # maybe a symlink
# set(LLVM_SRC_DIR2 "./llvm-3.6.0.src")  # maybe a symlink
# set(LLVM_SRC_DIR2 "./llvm-3.6.2.src")  # maybe a symlink
set(LLVM_SRC_DIR2 "./llvm-3.7.0.src")  # maybe a symlink
set(CLANG_SRC_DIR2 ${LLVM_SRC_DIR2}/tools/clang)
include_directories(${CLANG_SRC_DIR2}/lib/CodeGen ${CLANG_SRC_DIR2}/lib/ ${LLVM_SRC_DIR2}/lib/)
include_directories(${CLANG_SRC_DIR2}/include)

set(handby_lib_SRCS
#  llvm-3.5.0.src/tools/clang/lib/CodeGen/CodeGenModule.cpp
#  llvm-3.5.0.src/tools/clang/lib/CodeGen/CodeGenFunction.cpp
#    go/vmgo.cpp
    go/goinit.cpp
    go/marshallgo.cpp
    extinit.cpp
    ruby/rubyinit.cpp
    ruby_cxx.cpp
    marshallruby.cpp
    connectruby.cpp
    qtruby.cpp
    qtregister.cpp    
    qtobjectmanager.cpp
    entry.cpp
    ctrlengine.cpp
    ivm/callcontext.cpp
    ivm/codeunit.cpp
    ivm/clvmengine.cpp
    ivm/modulemanager.cpp
    operatorengine.cpp
    ivm/findsymbolvisitor.cpp    
    compilerengine.cpp
    ivm/fecache.cpp    
    frontengine.cpp
    clvm_operator.cpp
    clvm.cpp
    clvm_letacy.cpp
    ivm/clvmjitlistener.cpp
    ivm/misc.cpp
#    namelookup.cpp
    callargument.cpp
    invokestorage.cpp
#   metalize/metar_qt.cpp
#  metalize/metar_classes_qtcore.cpp
#  metalize/metar_classes_qtgui.cpp
#  metalize/metar_classes_qtwidgets.cpp
#  metalize/metar_classes_qtnetwork.cpp
#  rcoreapplication.cpp
#  functionize/kitech_qtcore.cpp
#  metalize/kitech_qtcore.cpp
#  metalize/kitech_qtnetwork.cpp
#  metalize/metas.cpp
    debugoutput.cpp
    macrolib.cpp
)

set(handby_exe_SRCS
    ${handby_lib_SRCS}
    main.cpp
    block.cpp
)

# 还是全静态编译靠谱些。
# set(CLANGLIBS  -lclang -lLLVM-3.7
set(CLANGLIBS
    -lclangTooling
    -lclangFrontendTool
    -lclangFrontend
    -lclangDriver
    -lclangSerialization
    -lclangCodeGen
    -lclangParse
    -lclangSema
    -lclangStaticAnalyzerFrontend
    -lclangStaticAnalyzerCheckers
    -lclangStaticAnalyzerCore
    -lclangAnalysis
    -lclangARCMigrate
    -lclangRewriteFrontend
    -lclangRewrite
    -lclangEdit
    -lclangASTMatchers
    -lclangAST
    -lclangLex
    -lclangBasic

    -lncurses  # 如果不加这个的话，出现undefined symbol: setupterm，可是在哪儿用的呢
    # below from llvm-config --libs
    ${LLVM_LIBS_OUTPUT}
)

add_library(handby SHARED ${handby_lib_SRCS})
target_link_libraries(handby profiler ${RUBY_LIBRARY} ${CLANGLIBS})
# target_link_libraries(handby ruby -L./myllvm/lib  ${CLANGLIBS})
qt5_use_modules(handby Core Gui Widgets Network)
# qt5_use_modules(handby Core Gui Widgets Network WebSockets)
#set_target_properties(handby PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "hdrsrc.h")
#set_target_properties(handby PROPERTIES COTIRE_UNITY_LINK_LIBRARIES_INIT "COPY")
#cotire(handby)

### need after make, thus is only backup
#exec_program(cp ARGS "-va libhandby.so  handby.so.bak" OUTPUT_VARIABLE RENAME_OUTPUT RETURN_VALUE RENAME_RET)
#message("renaming ..., ${RENAME_RET}, ${RENAME_OUTPUT}.")


# add_executable(ehandby ${handby_exe_SRCS} )
# target_link_libraries(ehandby ruby  ${CLANGLIBS})
# qt5_use_modules(ehandby Core Gui Widgets Network WebSockets)

# add_subdirectory(demos)
#add_subdirectory(uitools/rcc)
add_subdirectory(uitools/rbuic)
add_subdirectory(uitools/rbrcc)
