# CMake configuration for the heap2obj library
#
# COPYRIGHT (c) 2025 The Fellowship of SML/NJ (https://smlnj.org)
# All rights reserved.
#

if(${SMLNJ_ARCH_AMD64})
  set(target_arch X86)
elseif (${SMLNJ_ARCH_ARM64})
  set(target_arch AArch64)
endif()

# determine the LLVM libraries
llvm_map_components_to_libnames(LLVM_LIBS ${target_arch})

set(SRCS
  heap2obj.cpp)

add_library(heap2obj STATIC ${SRCS})

add_dependencies(heap2obj llvm-headers)

target_compile_definitions(heap2obj PRIVATE "-DSMLNJ_TARGET_TRIPLE=\"${LLVM_HOST_TRIPLE}\"")
target_compile_options(heap2obj PRIVATE -fno-exceptions -fno-rtti)
target_include_directories(heap2obj PRIVATE
  ${SMLNJ_LLVM_BINARY_DIR}/smlnj/include
  ${SMLNJ_LLVM_BINARY_DIR}/llvm/include
  ${SMLNJ_LLVM_SOURCE_DIR}/llvm/include)

install(TARGETS heap2obj)
