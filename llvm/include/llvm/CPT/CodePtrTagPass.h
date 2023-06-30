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
#include "llvm/CPT/FuncTypeCfgPass.h"

using namespace llvm;
using namespace CPT;
using namespace std;

//#define DEBUG_TYPE "aos_reach_test_pass"

//namespace {
  class CodePtrTagPass : public ModulePass {

  public:
    static char ID; // Pass identification, replacement for typeid
    CodePtrTagPass() : ModulePass(ID) {}

    set<string> black_set;
  	set<Function*> white_set;
  	map<Type*, set<Type*>*> type_alias;
    map<Function*, map<CallInst*,set<Function*>*>*> edge_map;
  	set<Value*> visit_set;

  	map<Type*,u_int16_t> label_map;
  	u_int16_t glb_label = 0;

		LLVMContext *C = nullptr;
		const DataLayout *DL = nullptr;
		Function *main = nullptr;
		Function *entry = nullptr;

    bool runOnModule(Module &M) override;
    void getAnalysisUsage(AnalysisUsage &AU) const;

    bool Baseline = false;
    bool CPT = false;
		bool QEMU = false;

    u_int16_t max_label = 0;
    unsigned statNumIndirectCall = 0;
    unsigned statNumESTR = 0;
    unsigned statNumFunction = 0;
    unsigned statMaxEqClassSize = 0;
    unsigned temp_cnt = 0;
    unsigned func_num = 0;

    std::map<const Type *, uint64_t> TypeIDCache;
    char *voidptr = nullptr;

  private:  
		void initEMT(Module &M);
    void replaceUsers(Module &M);
    void replaceUser(Module &M, Instruction *pI, Value *pV, Value *pF);
    void replaceUserOfOp(Module &M, Operator *pOp, Instruction *pI);
    void handleConstant(Module &M, Constant *pConst, Value *pV);
    void handleBitCastOperator(Module &M, BitCastOperator *pBC, Function *pF);
    void handlePtrToIntOperator(Module &M, PtrToIntOperator *pPTI, Function *pF);
    void handlePHINode(Module &M, PHINode *pPN, Value *pV, Value *pF);
    void handleGlobalFuncPtr(Module &M, GlobalVariable *pGV, Value *pV);
    void handleGlobalVariables(Module &M);
    void handleGlobalVariable(Module &M, GlobalVariable *pGV);
    void handleFunctionTy(Module &M, Value *pV, GlobalVariable *pGV, vector<Value*> *indices, Value *pF);
    void handleArrayTy(Module &M, Constant *pConst, GlobalVariable *pGV, vector<Value*> *indices);
    void handleStructTy(Module &M, Constant *pConst, GlobalVariable *pGV, vector<Value*> *indices);
		void handleIndirectCalls(Module &M);
		void handleIndirectCall(Module &M, Instruction *pI);
		void init(Module &M);
    void printFuncAddr(Module &M);
    void insertCptSet(Module &M);
    bool isIndirectCall(Instruction *pI);
    Instruction *insertTagc(Module &M, IRBuilder<> *Builder, Value *pV, Value *pF);
    Instruction *insertXtag(Module &M, IRBuilder<> *Builder, Value *pV);
    void insertEact(Module &M, IRBuilder<> *Builder, Value *pV, size_t label);
    void insertEstr(Module &M, IRBuilder<> *Builder, Value *pV, size_t label);
    void replaceOp(Value *pVa, Value *pVb, Instruction *pI);
    void handleIntrinsicFunctions(Module &M);
    void buildTypeString(const Type *T, llvm::raw_string_ostream &O);
    uint64_t getTypeIDFor(const Type *T);
    Constant *getTypeIDConstantFrom(const Type &T, LLVMContext &C);
  };
//}
