/// \file object-file.hpp
///
/// \copyright 2024 The Fellowship of SML/NJ (https://www.smlnj.org)
/// All rights reserved.
///
/// \brief A wrapper around LLVM's JIT infrastructure that we use to extract the
///        executable code for the CFG.
///
/// \author John Reppy
///

#ifndef _OBJECT_FILE_HPP_
#define _OBJECT_FILE_HPP_

#include "llvm/Support/raw_ostream.h"
#include <cstdint>
#include "objfile-stream.hpp"

struct TargetInfo;

/******************** class ObjectFile ********************/

class ObjectFile {
public:

    ObjectFile () = delete;
    ObjectFile (ObjectFile &) = delete;

    /// create an object-file object from the output of the code generator
    /// \param output  the output stream that the code was generated into
    explicit ObjectFile (ObjfileStream const &output);

    ~ObjectFile ();

private:
    struct TargetInfo const *_target;
    class MemManager *_memManager;

}; // ObjectFile

#endif //! _OBJECT_FILE_HPP_
