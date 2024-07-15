# CMake configuration for the heap2obj tool
#
# COPYRIGHT (c) 2024 The Fellowship of SML/NJ (https://smlnj.org)
# All rights reserved.
#

if(${ARCH_AMD64})
  set(target_arch X86)
elseif (${ARCH_ARM64})
  set(target_arch AArch64)
endif()

# determine the LLVM libraries
llvm_map_components_to_libnames(LLVM_LIBS ${target_arch})

set(SRCS
  main.cpp)

add_executable(heap2obj ${SRCS})
add_dependencies(heap2obj llvm-headers)

target_compile_definitions(heap2obj PRIVATE "-DSMLNJ_TARGET_TRIPLE=\"${LLVM_HOST_TRIPLE}\"")
target_compile_options(heap2obj PRIVATE -fno-exceptions -fno-rtti)
target_include_directories(heap2obj PRIVATE
  ${SMLNJ_LLVM_BINARY_DIR}/llvm/include
  ${SMLNJ_LLVM_SOURCE_DIR}/llvm/include)
target_link_libraries(heap2obj ${LLVM_LIBS})

install(TARGETS heap2obj)
