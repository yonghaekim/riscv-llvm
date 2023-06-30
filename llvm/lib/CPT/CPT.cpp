#include "llvm/CPT/CPT.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include <regex>
#include <map>

using namespace llvm;
using namespace CPT;

static cl::opt<CPT::CptType> CptInstType(
    "cpt-type", cl::init(CptType::None),
    cl::desc("Code-pointer tagging"),
    cl::value_desc("mode"),
    cl::values(clEnumValN(CptType::None, "base", "Disable protection"),
               clEnumValN(CptType::CPT, "cpt", "Enable code-pointer tagging")));

CptType CPT::getCptInstType() {
  return CptInstType;
}

static cl::opt<CPT::CptQemuEn> CptQemuMode(
    "cpt-qemu", cl::init(CptQemuEn::Disable),
    cl::desc("CPT QEMU mode control"),
    cl::value_desc("mode"),
    cl::values(clEnumValN(CptQemuEn::Disable, "disable", "Disable CPT QEMU mode"),
               clEnumValN(CptQemuEn::Enable, "enable", "Enable CPT QEMU mode")));

CptQemuEn CPT::getCptQemuMode() {
  return CptQemuMode;
}
