/*! \file config.h
 **
 ** \copyright 2024 The Fellowship of SML/NJ (https://www.smlnj.org)
 ** All rights reserved.
 **
 ** \brief Configuration parameters as determined by CMake.  This file is
 **        used when the repository is compiled as a top-level project.
 **
 ** \author John Reppy
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

/* Code generation targets */
#cmakedefine ENABLE_ARM64
#cmakedefine ENABLE_X86

/* Host architecture parameters */
#cmakedefine ARCH_AMD64
#cmakedefine ARCH_ARM64

/* Host operating system parameters */
#cmakedefine OPSYS_DARWIN
#cmakedefine OPSYS_LINUX

#endif /* !_CONFIG_H_ */
