project(handby)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_VERBOSE_MAKEFILE on)

# add_definitions(-g -O2 -std=c++11) # -std=c++14 -fno-rtti)  # -fno-rtti
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-g -O2 -std=c++11 -std=c++14")

set(CMAKE_INCLUDE_CURRENT_DIR on)
set(CMAKE_AUTOMOC on)
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Network)
find_package(Qt5WebSockets)

find_package(Ruby)
message("ruby?? ${RUBY_FOUND}, ${RUBY_INCLUDE_DIRS}, ${RUBY_LIBRARY}")
include_directories(${RUBY_INCLUDE_DIRS})

exec_program(llvm-config ARGS --cxxflags 
  OUTPUT_VARIABLE CLANG_CXXFLAGS_OUTPUT RETURN_VALUE CLANG_CXXFLAGS_VALUE)
message("clang defs: ${CLANG_CXXFLAGS_OUTPUT}")
# message(${CLANG_CXXFLAGS_VALUE})
#add_definitions(${CLANG_CXXFLAGS_OUTPUT} )
#set(COMPILER_FLAGS ${CLANG_CXXFLAGS_OUTPUT})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_CXXFLAGS_OUTPUT}")

set(handby_lib_SRCS
  qom.cpp
  entry.cpp
  clvm.cpp
  debugoutput.cpp
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
target_link_libraries(handby ruby  ${CLANGLIBS})
qt5_use_modules(handby Core Gui Widgets Network WebSockets)

### need after make, thus is only backup
#exec_program(cp ARGS "-va libhandby.so  handby.so.bak" OUTPUT_VARIABLE RENAME_OUTPUT RETURN_VALUE RENAME_RET)
#message("renaming ..., ${RENAME_RET}, ${RENAME_OUTPUT}.")


# add_executable(ehandby ${handby_exe_SRCS} )
# target_link_libraries(ehandby ruby  ${CLANGLIBS})
# qt5_use_modules(ehandby Core Gui Widgets Network WebSockets)

