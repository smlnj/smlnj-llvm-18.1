/// \file object-file.cpp
///
/// \copyright 2024 The Fellowship of SML/NJ (https://www.smlnj.org)
/// All rights reserved.
///
/// \brief A wrapper around LLVM's JIT infrastructure that we use to extract the
///        executable code for the CFG.
///
/// \author John Reppy
///

#include "object-file.hpp"
#include "llvm/Support/Error.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/MemoryBufferRef.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include <cstdint>

#include <iostream>  // for debugging

namespace smlnj {
namespace cfgcg {

inline uintptr_t alignBy (uintptr_t n, uintptr_t align)
{
    return (n + (align - 1)) & ~(align - 1);
}
inline uintptr_t alignBy (uintptr_t n, llvm::Align align)
{
    return alignBy (n, uintptr_t(align.value()));
}

/***** class MemManager *****/

class MemManager : public llvm::RTDyldMemoryManager
{
public:
    // constructor
    MemManager () : _base(nullptr), _code(), _roData() { }

    virtual ~MemManager () override
    {
        if (this->_base != nullptr) {
            delete[] this->_base;
        }
    }

    /// we want to put all of the sections together in a single chunk of memory, so
    /// we tell the JIT linker that it needs to call `reserveAllocationSpace`.
    virtual bool needsToReserveAllocationSpace () override { return true; }

    virtual void reserveAllocationSpace (
        uintptr_t codeSzb,
        llvm::Align codeAlign,
        uintptr_t roDataSzb,
        llvm::Align roDataAlign,
        uintptr_t rwDataSzb,
        llvm::Align rwDataAlign) override;

    virtual uint8_t *allocateCodeSection (
        uintptr_t szb,
        unsigned align,
        unsigned secID,
        llvm::StringRef name) override;

    virtual uint8_t *allocateDataSection (
        uintptr_t szb,
        unsigned align,
        unsigned secID,
        llvm::StringRef name,
        bool readOnly) override;

    void registerEHFrames (uint8_t *addr, uint64_t loadAddr, size_t numBytes) override;

    virtual bool finalizeMemory (std::string* ErrMsg = nullptr) override
    {
        // nothing to do here, since we are not going to execute the code
        return true;
    }

    /// return the base address of the memory object
    uint8_t const *data () const
    {
        assert (this->_base != nullptr && "no memory allocated");
        return this->_base;
    }

    /// return the total size of the memory object
    size_t size () const
    {
        return this->_code._paddedSzb + this->_roData._paddedSzb;
    }

private:
    /// information about an in-memory section from the object file
    struct _Section {
        uint8_t *_data;         ///< base address of section
        size_t _szb;            ///< current size of code/data in section
        size_t _paddedSzb;      ///< padded size of the section

        /// constructor
        _Section () : _data(nullptr), _szb(0) { }

        /// set the padded size of the section
        void setSize (size_t szb, llvm::Align align)
        {
            this->_paddedSzb = alignBy(szb, align);
        }

        /// allocate memory in the section
        uint8_t *alloc (size_t nb, unsigned align)
        {
            assert (this->_data != nullptr && "no memory allocated for section");
            assert (alignBy(this->_szb, align) + alignBy(nb, align) <= this->_paddedSzb
                && "insufficient space for allocation");

            uint8_t *ptr = this->_data + alignBy(this->_szb, align);
            this->_szb = (ptr - this->_data) + alignBy(nb, align);
llvm::dbgs() << "## alloc " << nb << " bytes at " << (void *)ptr << "\n";
            return ptr;
        }
    };

    uint8_t *_base;             ///< pointer to memory
    _Section _code;             ///< code section
    _Section _roData;           ///< read-only data section
    _Section _rwData;           ///< read-write data section

}; // class MemManager

void MemManager::reserveAllocationSpace (
    uintptr_t codeSzb,
    llvm::Align codeAlign,
    uintptr_t roDataSzb,
    llvm::Align roDataAlign,
    uintptr_t rwDataSzb,
    llvm::Align rwDataAlign)
{
/*DEBUG*/
llvm::dbgs() << "# reserve: codeSzb = " << codeSzb << ", roData = " << roDataSzb
<< ", rwData = " << rwDataSzb << "\n";
/*DEBUG*/

//    assert (rwDataSzb == 0 && "unexpected non-empty RW data");
    assert (this->_base == nullptr && "memory already allocated");

    // set the section sizes with padding for alignment.  Since we are going to
    // concatenate the sections, we need to pad the code section by the greatest
    // of the alignments
    if (roDataAlign > codeAlign) { codeAlign = roDataAlign; }
    if (rwDataAlign > codeAlign) { codeAlign = rwDataAlign; }
    this->_code.setSize(codeSzb, codeAlign);
    this->_roData.setSize(roDataSzb, roDataAlign);
    this->_rwData.setSize(rwDataSzb, rwDataAlign);

    /// allocate the memory for the in-memory sections
    this->_base = new uint8_t (this->size());

    /// assign base adresses for the sections
    this->_code._data = this->_base;
    this->_roData._data = this->_base + this->_code._paddedSzb;
    this->_rwData._data = this->_roData._data + this->_roData._paddedSzb;

} // MemManager::reserveAllocationSpace

uint8_t *MemManager::allocateCodeSection (
    uintptr_t szb,
    unsigned align,
    unsigned secID,
    llvm::StringRef name)
{
    assert (this->_base != nullptr && "memory has not been reserved");

/*DEBUG*/
llvm::dbgs() << "# allocate code[" << name << "]: szb = " << szb
<< "; align = " << align << "\n";
/*DEBUG*/

    return this->_code.alloc(szb, align);

} // MemManager::allocateCodeSection

uint8_t *MemManager::allocateDataSection (
    uintptr_t szb,
    unsigned align,
    unsigned secID,
    llvm::StringRef name,
    bool readOnly)
{
    assert (this->_base != nullptr && "memory has not been reserved");
//    assert (readOnly && "unexpected allocation of RW data");

/*DEBUG*/
llvm::dbgs() << "# allocate data[" << name << "]: szb = " << szb
<< "; align = " << align << "; ro = " << readOnly << "\n";
/*DEBUG*/

    if (readOnly) {
        return this->_roData.alloc(szb, align);
    } else {
        return this->_rwData.alloc(szb, align);
    }

} // MemManager::allocateDataSection

void MemManager::registerEHFrames (uint8_t *addr, uint64_t loadAddr, size_t numBytes)
{
/*DEBUG*/
llvm::dbgs() << "# register EH frames: addr = " << (void*)addr
<< ", loadAddr = " << loadAddr << "; numBytes = " << numBytes << "\n";
/*DEBUG*/
} // MemManager::registerEHFrames


/***** class SymbolResolver *****/

// Create the LLVM object loader.
class SymbolResolver : public llvm::JITSymbolResolver
{
public:

    SymbolResolver () { }
    virtual ~SymbolResolver () override { }

    virtual void lookup (
        llvm::JITSymbolResolver::LookupSet const &syms,
        llvm::JITSymbolResolver::OnResolvedFunction orf
    ) override
    {
        /* there should not be any symbols to look up, since
         * the generated LLVM is closed.
         */
        assert (false && "unexpected external symbol lookup");
    }

    virtual llvm::Expected<LookupSet> getResponsibilitySet (
        const llvm::JITSymbolResolver::LookupSet & symbols
    ) override
    {
        return llvm::JITSymbolResolver::LookupSet(); /* return empty set */
    }

private:

}; /* class SymbolResolver */


/***** class ObjectFile *****/

ObjectFile::ObjectFile (ObjfileStream const &output)
: _memManager(new MemManager)
{
    auto bytes = output.getData();

    std::unique_ptr<llvm::object::ObjectFile> object
        = cantFail(
            llvm::object::ObjectFile::createObjectFile(
                llvm::MemoryBufferRef(bytes, "object-file memory")));

    SymbolResolver symbolResolver;
    llvm::RuntimeDyld loader(*this->_memManager, symbolResolver);

/*DEBUG*/
llvm::dbgs() << "# link object file\n";
/*DEBUG*/

    // Use the LLVM object loader to load the object.
    std::unique_ptr<llvm::RuntimeDyld::LoadedObjectInfo> loadedObject =
        loader.loadObject(*object);
    loader.finalizeWithMemoryManagerLocking();
    if (loader.hasError()) {
        std::string errMsg =
            std::string("RuntimeDyld failed: ") + loader.getErrorString().data();
        llvm::report_fatal_error (llvm::StringRef(errMsg), false);
     }

}

ObjectFile::~ObjectFile ()
{
    delete this->_memManager;
}

size_t ObjectFile::size () const
{
    return this->_memManager->size();
}

uint8_t const *ObjectFile::data () const
{
    return this->_memManager->data();
}

} // namespace cfgcg
} // namespace smlnj
