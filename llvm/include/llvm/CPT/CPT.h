#ifndef LLVM_CPT_H
#define LLVM_CPT_H

#include "llvm/IR/Constant.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"

namespace llvm {
namespace CPT {

enum CptType {
  None,
  CPT
};

enum CptQemuEn {
	Disable,
	Enable
};

CptType getCptInstType();
CptQemuEn getCptQemuMode();
Pass *createFuncTypeCfgPass();
Pass *createCodePtrTagPass();
}
} // llvm

#endif //LLVM_CPT_H
