#include "llvm/DPT/DPT.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include <regex>
#include <map>

using namespace llvm;
using namespace DPT;

//static cl::opt<bool> EnableFuncCounter("wyfy-count", cl::Hidden,
//                                      cl::desc("DPT function counter pass"),
//                                      cl::init(false));

static cl::opt<DPT::DptType> DptInstType(
    "dpt-type", cl::init(DptType::None),
    cl::desc("DPT inst type"),
    cl::value_desc("mode"),
    cl::values(clEnumValN(DptType::None, "base", "Disable protection"),
               clEnumValN(DptType::DPT_H, "dpt-h", "Enable heap-only protection"),
               clEnumValN(DptType::DPT_C, "dpt-c", "Enable coarse-grained protection"),
               clEnumValN(DptType::DPT_F, "dpt-f", "Enable fine-grained protection")));

DptType DPT::getDptInstType() {
  return DptInstType;
}

static cl::opt<DPT::DptQemuEn> DptQemuMode(
    "dpt-qemu", cl::init(DptQemuEn::Disable),
    cl::desc("DPT QEMU mode control"),
    cl::value_desc("mode"),
    cl::values(clEnumValN(DptQemuEn::Disable, "disable", "Disable DPT QEMU mode"),
               clEnumValN(DptQemuEn::Enable, "enable", "Enable DPT QEMU mode")));

DptQemuEn DPT::getDptQemuMode() {
  return DptQemuMode;
}

//bool DPT::isEnableFuncCounter() {
//  return EnableFuncCounter;
//}
