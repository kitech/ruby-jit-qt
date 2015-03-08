project(handby)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_VERBOSE_MAKEFILE on)

# add_definitions(-g -O2 -std=c++11) # -std=c++14 -fno-rtti)  # -fno-rtti
add_definitions(-DQT_MESSAGELOGCONTEXT=1 -DLLVM_DUMP_COLOR=1)
set(CMAKE_C_COMPILER ./myllvm/bin/clang)
set(CMAKE_CXX_COMPILER ./myllvm/bin/clang++)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
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
find_package(Qt5WebSockets)

find_package(Ruby)
message("ruby?? ${RUBY_FOUND}, ${RUBY_INCLUDE_DIRS}, ${RUBY_LIBRARY}")
include_directories(${RUBY_INCLUDE_DIRS} ${Qt5Core_PRIVATE_INCLUDE_DIRS})

exec_program(llvm-config ARGS --cxxflags 
  OUTPUT_VARIABLE CLANG_CXXFLAGS_OUTPUT RETURN_VALUE CLANG_CXXFLAGS_VALUE)
message("clang defs: ${CLANG_CXXFLAGS_OUTPUT}")
# message(${CLANG_CXXFLAGS_VALUE})
#add_definitions(${CLANG_CXXFLAGS_OUTPUT} )
#set(COMPILER_FLAGS ${CLANG_CXXFLAGS_OUTPUT})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_CXXFLAGS_OUTPUT}")

### fix c++11 => c++14
string(REPLACE "-std=gnu++11" "-std=c++14" NEW_FLAGS ${CMAKE_CXX_FLAGS})
string(REPLACE "-std=c++11" "-std=c++14" NEW_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${NEW_FLAGS}")

set(LLVM_SRC_DIR2 "./llvm-3.5.1.src")  # maybe a symlink
set(CLANG_SRC_DIR2 ${LLVM_SRC_DIR2}/tools/clang)
include_directories(${CLANG_SRC_DIR2}/lib/CodeGen ${CLANG_SRC_DIR2}/lib/)

set(handby_lib_SRCS
#  llvm-3.5.0.src/tools/clang/lib/CodeGen/CodeGenModule.cpp
#  llvm-3.5.0.src/tools/clang/lib/CodeGen/CodeGenFunction.cpp
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
    clvmengine.cpp
    operatorengine.cpp
    ivm/findsymbolvisitor.cpp    
    compilerengine.cpp
    ivm/fecache.cpp    
    frontengine.cpp
    clvm_operator.cpp
    clvm.cpp
    namelookup.cpp
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

set(CLANGLIBS  -lclang -lLLVM-3.5
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
)

add_library(handby SHARED ${handby_lib_SRCS})
target_link_libraries(handby profiler ruby  ${CLANGLIBS})
# target_link_libraries(handby ruby -L./myllvm/lib  ${CLANGLIBS})
qt5_use_modules(handby Core Gui Widgets Network WebSockets)
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