/// \file main.cpp
///
/// \copyright 2024 The Fellowship of SML/NJ (https://www.smlnj.org)
/// All rights reserved.
///
/// \brief The heap2objfile tool converts a SML/NJ heap-image file to a host
///        object file that can be linked against the runtime system.
///
/// \author John Reppy
///

#include <string_view>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <cstdint>

namespace fs = std::filesystem;

#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"

/// different output targets
enum class Output {
    ObjFile,            ///< generate an object (".o") file
    AsmFile,            ///< generate native assembly in a ".s" file
    LLVMAsmFile         ///> generate LLVM assembly in a ".ll" file
};

// convert an integer to the tagged SML representation
//
inline uint64_t toSML (uint64_t n) { return n+n+1; }

[[noreturn]] void usage ()
{
    std::cerr << "usage: heap2objfile [ options ] <heap-file>\n";
    std::cerr << "options:\n";
    std::cerr << "    -o <obj-file>     -- specify the name of the output file\n";
    std::cerr << "    -S                -- emit assembly code to the output file\n";
    std::cerr << "    -emit-llvm        -- emit generated LLVM assembly to the output file\n";
    exit (1);
}

int main (int argc, char const **argv)
{
    Output out = Output::ObjFile;
    std::string src = "";
    std::string_view dst = "";

    // process command-line arguments
    std::vector<std::string_view> args(argv+1, argv+argc);

    if (args.empty()) {
	usage();
    }

    for (int i = 0;  i < args.size();  i++) {
	if (args[i][0] == '-') {
	    if (args[i] == "-o") {
		out = Output::ObjFile;
		i++;
		if (i < args.size()) {
		    dst = args[i];
		} else {
		    usage();
		}
	    } else if (args[i] == "-S") {
		out = Output::AsmFile;
	    } else if (args[i] == "--emit-llvm") {
                out = Output::LLVMAsmFile;
	    } else {
		usage();
	    }
	}
	else if (i == args.size()-1) { // last argument
	    src = args[i];
	}
    }
    if (src.empty()) {
        usage();
    }

/* TODO: process source file name to remove suffix */

    // load the heap-image data
    std::error_code ec;
    auto srcPath = fs::path(src);
    auto srcSts = fs::status(srcPath, ec);
    if (ec) {
        /* TODO: error */
    }
    auto fileSzb = fs::file_size(srcPath, ec);
    if (ec) {
        /* TODO: error */
    }
    uint64_t nBytes = (fileSzb + 7) & ~7; // round to 64-byte alignment
    std::vector<uint8_t> heapData = std::vector<uint8_t>(nBytes, 0);
    {
        std::basic_ifstream<uint8_t>
            srcS(srcPath, std::ios_base::in | std::ios_base::binary);
        srcS.read(heapData.data(), fileSzb);
        if (srcS.gcount() != fileSzb) {
            /* TODO: error */
        }
        srcS.close();
    }

    // create the LLVM context
    llvm::LLVMContext cxt;

    // get the integer types that we need
    auto i8Ty = llvm::IntegerType::get (cxt, 8);
    auto i64Ty = llvm::IntegerType::get (cxt, 64);
    auto heapTy = llvm::StructType::create (cxt, {
            i64Ty,
            llvm::ArrayType::get(i8Ty, nBytes)
        });

    llvm::Module module(src, cxt);

    // create the constants
    llvm::Constant *mSize = llvm::ConstantInt::get(i64Ty, toSML(nBytes));
    llvm::Constant *mData = llvm::ConstantDataArray::getRaw(
        llvm::StringRef(reinterpret_cast<char *>(heapData.data()), nBytes),
        nBytes,
        i8Ty);
    llvm::Constant *heapStruct = llvm::ConstantStruct::get (
        heapTy,
        { mSize, mData });

    auto heapV = new llvm::GlobalVariable (
        module,
        true,
        llvm::GlobalValue::ExternalLinkage,
        heapStruct,
        "_sml_heap_image");

    // output the object file

}
