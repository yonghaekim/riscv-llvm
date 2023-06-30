#include "llvm/Transforms/Scalar.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/CPT/CPT.h"
#include "llvm/Support/CommandLine.h"
#include <regex>
#include <map>
#include <iostream>

using namespace llvm;
using namespace CPT;
using namespace std;

//namespace {
  class FuncTypeCfgPass : public ModulePass {

  public:
    static char ID; // Pass identification, replacement for typeid
    FuncTypeCfgPass() : ModulePass(ID) {}
    bool runOnModule(Module &M) override;
    map<Type*,u_int16_t> getLabelMap();
    set<Function*> getWhiteSet();
    u_int16_t getMaxLabel();

    set<string> black_set;
  	set<Function*> white_set;
  	map<Type*,u_int16_t> label_map;

  private:  
    char *voidptr = nullptr;
    list<set<Type*>*> typeset_list;
    map<Type*,set<Type*>*> typeset_map;
		LLVMContext *C = nullptr;
		const DataLayout *DL = nullptr;

    void initBlackSet();
    void findWhiteSet(Module &M);
    void findTypeAlias(Module &M);
		void initLabelMap(Module &M);
    void addTypeAlias(Type *src_ty, Type *dst_ty);
    void addArgAlias(Type *src_ety, Type *dst_ety);
    bool doTypeComparison(FunctionType *func_ty, FunctionType *callee_ty);
		void init(Module &M);
    void add_typeset(Type *ty0, Type *ty1);
    void merge_typeset(Type *ty0, Type *ty1);
		void handleIndirectCalls(Module &M);
		void handleIndirectCall(Module &M, Instruction *pI);
    bool isIndirectCall(Instruction *pI);

    u_int16_t max_label = 0;
    unsigned statNumIndirectCall = 0;
    unsigned statNumESTR = 0;
    unsigned statMaxEqClassSize = 0;
  };
//}
