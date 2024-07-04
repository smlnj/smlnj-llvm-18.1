# CMake configuration for SML/NJ LLVM with JWA patches
#
# COPYRIGHT (c) 2024 The Fellowship of SML/NJ (http://www.smlnj.org)
# All rights reserved.
#

# When the project is not a sub-project, generate a placeholder
# "smlnj/config.h" file
#
if(PROJECT_IS_TOP_LEVEL)
  # determine the operating system
  #
  if (APPLE)
    set(OPSYS_DARWIN ON)
  elseif (LINUX)
    set(OPSYS_LINUX ON)
  else ()
    message(FATAL_ERROR "unsupported operating system")
  endif()

  # determine the archicture
  #
  if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(ARCH_AMD64 ON)
  elseif (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "arm64")
    set(ARCH_ARM64 ON)
  else ()
    message(FATAL_ERROR "unsupported architecture ${CMAKE_HOST_SYSTEM_PROCESSOR}")
  endif()

  # determine the enabled targets
  #
  foreach(T IN LISTS LLVM_TARGETS_TO_BUILD)
    if(${T} STREQUAL "X86")
      message(STATUS "enable X86 target")
      set(ENABLE_X86 ON)
    elseif(${T} STREQUAL "AArch64")
      message(STATUS "enable ARM64 target")
      set(ENABLE_ARM64 ON)
    else()
      message(FATAL_ERROR "unknown target architecture ${T}")
    endif()
  endforeach()

  set(include_dir ${CMAKE_CURRENT_BINARY_DIR}/include/smlnj)

  # generate the config.h file
  #
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config/config.h.cmake
    ${include_dir}/config.h
    @ONLY)

  # arrange for the config.h file to be installed
  install(DIRECTORY ${include_dir} TYPE INCLUDE)
endif()

include_directories(include)
add_subdirectory(include)
add_subdirectory(lib)

if (SMLNJ_CFGC_BUILD)
  message(STATUS "cfgc tool enabled.")
  add_subdirectory(cfgc)
endif()
