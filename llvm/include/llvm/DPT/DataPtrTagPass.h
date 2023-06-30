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
#include "llvm/DPT/DPT.h"
#include "llvm/Support/CommandLine.h"
#include <regex>
#include <map>
#include <iostream>

using namespace llvm;
using namespace DPT;
using namespace std;

//#define DEBUG_TYPE "aos_reach_test_pass"

//namespace {
  class DataPtrTagPass : public ModulePass {

  public:
    static char ID; // Pass identification, replacement for typeid
    DataPtrTagPass() : ModulePass(ID) {}

//    map<Value *, DptNode *> value_map; 
//    //set<DptNode *> visit_set;
//		list<Instruction *> inst_list;
//
//		set<DptNode *> taint_nodes;
//		map<Type *, set<unsigned>> taint_indices;
//		map<Type *, set<unsigned>> signed_indices;
//		set<Value *> sign_set;
//		list<vector<Value *>> indices_list;
    set<GetElementPtrInst *> gep_visit_set;
    set<GetElementPtrInst *> gep_erase_set;

		LLVMContext *C = NULL;
		const DataLayout *DL = NULL;

    bool runOnModule(Module &M) override;
		//void getAnalysisUsage(AnalysisUsage &AU) const;

    bool Baseline = false;
    bool DPT_H = false;
    bool DPT_C = false;
    bool DPT_F = false;
		bool QEMU = false;

    unsigned statNumGV = 0;
    unsigned statNumAI = 0;
    unsigned statNumCI = 0;
    unsigned statNumGEP = 0;

    unsigned temp_cnt = 0;
    unsigned intr_num = 0;
    unsigned func_num = 0;


    std::map<const Type *, uint64_t> TypeIDCache;

  private:  
    //void handleCmdLineArguments(Module &M);
		void handleGlobalVariables(Module &M);
		void handleGlobalVariable(Module &M, GlobalVariable *pGV);
		void handleOperators(Module &M, GlobalVariable *pGV);
		void handleAllocas(Module &M);
		void handleAlloca(Module &M, AllocaInst *pAI);
		void handleMallocs(Module &M);
		void handleMalloc(Module &M, Instruction *pI);
		void handleFree(Module &M, Instruction *pI);

		void handleGEPs(Module &M);
		void handleGEP(Module &M, GetElementPtrInst *pGEP);
    void handlePtrToInts(Module &M);
		void handleQEMU(Module &M);
    void addXtag(Instruction *pI, unsigned idx, Module &M);

		//void handleCmdLineArguments(Module &M);
		bool handleInstructions(Module &M);
    //bool doReachabilityTest(DptNode *node);
		//bool handleAlloca(Function *pF, AllocaInst *pAI);
		bool handleDealloc(Function *pF, ReturnInst *pRI);
		bool handleMalloc(Function *pF, Instruction *pI);
		bool handleFree(Function *pF, Instruction *pI);
		//bool handleStruct(Function *pF, AllocaInst *pAI);
		//bool handleStruct(Function *pF, GlobalVariable *pGV);
		//bool handleStruct(Function *pF, Value *pV, Type *ty);
		void handleStruct(Function *pF, Value *pV, set<Type *> type_set);
		bool IsArrayTy(Type *ty);
		bool IsStructTy(Type *ty);
    bool IsStructTyWithArray(Type *ty);
    set<Type *> getStructTypes(Type *ty, set<Type *> type_set);
		//Type *GetStructTy(Type *ty);
		//unsigned GetStartIdx(Function *pF, Type *ty);
		//void printNode(DptNode *node);
		void init(Module &M);
		void handleElement(Function *pF, Value *pV, Type *ty, list<vector<Value *>> indices_list, bool isGV, bool isCI);
    void buildTypeString(const Type *T, llvm::raw_string_ostream &O);
    uint64_t getTypeIDFor(const Type *T);
    Constant *getTypeIDConstantFrom(const Type &T, LLVMContext &C);
		void insertSign(Function *pF, GetElementPtrInst *pGEP, Type *ty);
    void replaceUsesInFunction(Value *pV, Value *pNew, Function *pF);
    void insertDptSet(Module &M);
    void handleIntrinsicFunctions(Module &M);
    void handleIntrinsicFunction(Module &M, Instruction *pI);
    void addXpacm(Instruction *pI, unsigned idx, Module &M);
  };
//}

