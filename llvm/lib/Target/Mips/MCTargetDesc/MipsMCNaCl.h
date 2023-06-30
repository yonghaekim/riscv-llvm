//===-- MipsMCNaCl.h - NaCl-related declarations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSMCNACL_H
#define LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSMCNACL_H

#include "llvm/MC/MCELFStreamer.h"

namespace llvm {

// Log2 of the NaCl MIPS sandbox's instruction bundle size.
static const unsigned MIPS_NACL_BUNDLE_ALIGN = 4u;

bool isBasePlusOffsetMemoryAccess(unsigned Opcode, unsigned *AddrIdx,
                                  bool *IsStore = nullptr);
bool baseRegNeedsLoadStoreMask(unsigned Reg);

// This function creates an MCELFStreamer for Mips NaCl.
MCELFStreamer *createMipsNaClELFStreamer(MCContext &Context,
                                         std::unique_ptr<MCAsmBackend> TAB,
                                         std::unique_ptr<MCObjectWriter> OW,
                                         std::unique_ptr<MCCodeEmitter> Emitter,
                                         bool RelaxAll);
}

#endif
