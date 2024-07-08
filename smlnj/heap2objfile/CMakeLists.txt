# CMake configuration for the heap2objfile tool
#
# COPYRIGHT (c) 2024 The Fellowship of SML/NJ (https://smlnj.org)
# All rights reserved.
#

# determine the LLVM libraries
llvm_map_components_to_libnames(LLVM_LIBS ${LLVM_TARGET_ARCH})

set(SRCS
  main.cpp)

add_executable(heap2objfile ${SRCS})
add_dependencies(heap2objfile llvm-headers)

target_compile_options(heap2objfile PRIVATE -fno-exceptions -fno-rtti)
target_include_directories(heap2objfile PRIVATE
  ${CMAKE_BINARY_DIR}/llvm/include ${CMAKE_SOURCE_DIR}/llvm/include)
target_link_libraries(heap2objfile ${LLVM_LIBS})

install(TARGETS heap2objfile)
