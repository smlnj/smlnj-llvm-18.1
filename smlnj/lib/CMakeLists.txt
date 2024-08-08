# CMake configuration for the Codegen Library
#
# COPYRIGHT (c) 2024 The Fellowship of SML/NJ (https://smlnj.org)
# All rights reserved.
#

set(SRCS
  asdl-integer.cpp
  asdl.cpp
  cfg-codegen.cpp
  cfg-init.cpp
  cfg-prim-codegen.cpp
  cfg.cpp
  cm-registers.cpp
  context.cpp
  lambda-var.cpp
  mc-gen.cpp
  object-file.cpp
  objfile-stream.cpp
  overflow.cpp
  target-info.cpp)

add_library(CFGCodeGen STATIC ${SRCS})

add_dependencies(CFGCodeGen llvm-headers)

target_compile_options(CFGCodeGen PRIVATE -fno-exceptions -fno-rtti)
target_include_directories(CFGCodeGen PRIVATE
  ${SMLNJ_LLVM_BINARY_DIR}/smlnj/include
  ${SMLNJ_LLVM_BINARY_DIR}/llvm/include
  ${SMLNJ_LLVM_SOURCE_DIR}/llvm/include)

install(TARGETS CFGCodeGen)
