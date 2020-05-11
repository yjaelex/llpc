/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 **********************************************************************************************************************/
/**
 ***********************************************************************************************************************
 * @file  llpcElfWriter.h
 * @brief LLPC header file: contains declaration of LLPC ELF writing utilities.
 ***********************************************************************************************************************
 */
#pragma once

#include "vkgcElfReader.h"

// Forward declaration
namespace llvm {
namespace msgpack {
class MapDocNode;
}
} // namespace llvm

namespace Llpc {

using Vkgc::BinaryData;
using Vkgc::ElfNote;
using Vkgc::ElfPackage;
using Vkgc::ElfReader;
using Vkgc::ElfSectionBuffer;
using Vkgc::ElfSymbol;
using Vkgc::GfxIpVersion;
using Vkgc::InvalidValue;
using Vkgc::Result;

// Forward declaration
class Context;

// =====================================================================================================================
// Represents a writer for storing data to an ELF buffer.
//
// NOTE: It is a limited implementation, it is designed for merging two ELF binaries which generated by LLVM back-end.
template <class Elf> class ElfWriter {
public:
  typedef ElfSectionBuffer<typename Elf::SectionHeader> SectionBuffer;

  ElfWriter(GfxIpVersion gfxIp);

  ~ElfWriter();

  static void mergeSection(const SectionBuffer *section1, size_t section1Size, const char *prefixString1,
                           const SectionBuffer *section2, size_t section2Offset, const char *prefixString2,
                           SectionBuffer *newSection);

  static void mergeMetaNote(Context *context, const ElfNote *note1, const ElfNote *note2, ElfNote *newNote);

  static void updateMetaNote(Context *context, const ElfNote *note, ElfNote *newNote);

  Result ReadFromBuffer(const void *buffer, size_t bufSize);
  Result copyFromReader(const ElfReader<Elf> &reader);

  void updateElfBinary(Context *context, ElfPackage *pipelineElf);

  void mergeElfBinary(Context *context, const BinaryData *fragmentElf, ElfPackage *pipelineElf);

  // Gets the section index for the specified section name.
  int GetSectionIndex(const char *name) const {
    auto entry = m_map.find(name);
    return entry != m_map.end() ? entry->second : InvalidValue;
  }

  void setSection(unsigned secIndex, SectionBuffer *section);

  ElfSymbol *getSymbol(const char *symbolName);

  ElfNote getNote(Util::Abi::PipelineAbiNoteType noteType);

  void setNote(ElfNote *note);

  Result getSectionDataBySectionIndex(unsigned secIdx, const SectionBuffer **ppSectionData) const;

  Result getSectionData(const char *name, const void **ppData, size_t *dataLength) const;

  void GetSymbolsBySectionIndex(unsigned secIdx, std::vector<ElfSymbol *> &secSymbols);

  void writeToBuffer(ElfPackage *elf);

  unsigned getSymbolCount() const;

  void getSymbol(unsigned idx, ElfSymbol *symbol);

  unsigned getRelocationCount();

  void getRelocation(unsigned idx, Vkgc::ElfReloc *reloc);

  Result linkGraphicsRelocatableElf(const llvm::ArrayRef<ElfReader<Elf> *> &relocatableElfs, Context *context);

  Result linkComputeRelocatableElf(const ElfReader<Elf> &relocatableElf, Context *context);

private:
  ElfWriter(const ElfWriter &) = delete;
  ElfWriter &operator=(const ElfWriter &) = delete;

  static void mergeMapItem(llvm::msgpack::MapDocNode &destMap, llvm::msgpack::MapDocNode &srcMap, unsigned key);

  size_t getRequiredBufferSizeBytes();

  void calcSectionHeaderOffset();

  void assembleNotes();

  void assembleSymbols();

  void reinitialize();

  GfxIpVersion m_gfxIp;                  // Graphics IP version info (used by ELF dump only)
  typename Elf::FormatHeader m_header;   // ELF header
  std::map<std::string, unsigned> m_map; // Map between section name and section index

  std::vector<SectionBuffer> m_sections; // List of section data and headers
  std::vector<ElfNote> m_notes;          // List of Elf notes
  std::vector<ElfSymbol> m_symbols;      // List of Elf symbols

  int m_textSecIdx;   // Section index of .text section
  int m_noteSecIdx;   // Section index of .note section
  int m_relocSecIdx;  // Section index of relocation section
  int m_symSecIdx;    // Section index of symbol table section
  int m_strtabSecIdx; // Section index of string table section
};

} // namespace Llpc
