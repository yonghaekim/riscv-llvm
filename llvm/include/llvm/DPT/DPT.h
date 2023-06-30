#ifndef LLVM_DPT_H
#define LLVM_DPT_H

#include "llvm/IR/Constant.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"

namespace llvm {
namespace DPT {

enum DptType {
  None,
  DPT_H,
  DPT_C,
  DPT_F
};

enum DptQemuEn {
	Disable,
	Enable
};

DptType getDptInstType();
DptQemuEn getDptQemuMode();
Pass *createDataPtrTagPass();
}
} // llvm

#endif //LLVM_DPT_H
