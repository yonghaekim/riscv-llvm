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
  class CodePtrAliasPass : public ModulePass {

  public:
    static char ID; // Pass identification, replacement for typeid
    CodePtrAliasPass() : ModulePass(ID) {}

		class CptAlias;
		class CptNode;

    class CptAlias {
    public:
      Value *ptr;
      CptNode *node;

      CptAlias () { }

      CptAlias (Value *_ptr, CptNode *_node) {
        ptr = _ptr;
				node = _node;
      }

      Value *getPtr() { return ptr; }

      CptNode *getNode() { return node; }

      void setNode(CptNode *_node) { node = _node; }
    };

    class CptNode {
    public:
			Type *ty;
			unsigned start_idx;
			list<Value *> indices;
			map<Value *, CptAlias *> aliases;

			set<CptNode *> children;
			set<CptNode *> parents;

			CptNode *mem_user;
			set<CptNode *> mem_edges;

			bool tainted;

      CptNode () {
				start_idx = 0;
				mem_user = nullptr;
				tainted = false;
			}

			void addChild(CptNode *node) {
				children.insert(node);
			}

			void addParent(CptNode *node) {
				parents.insert(node);
			}

			void removeChild(CptNode *node) {
				children.erase(node);
			}

			void removeParent(CptNode *node) {
				parents.erase(node);
			}

			bool findAlias(Value *ptr) {
				return (aliases[ptr] == NULL) ? false : true;
			}

			void addAlias(CptAlias *alias) {
        //if (ty != alias->getPtr()->getType()) {
        //  alias->getPtr()->dump();
        //  alias->getPtr()->getType()->dump();
        //  ty->dump();
        //  assert(false);
        //}

        //assert(ty == alias->getPtr()->getType());
				aliases[alias->getPtr()] = alias;
			}

			void setMemUser(CptNode *node) {
				if (node != nullptr)
					assert(mem_user == nullptr);
				mem_user = node;
			}

			void addMemEdge(CptNode *node) {
				mem_edges.insert(node);
			}

			void removeMemEdge(CptNode *node) {
				mem_edges.erase(node);
			}

			bool isTainted() { return tainted; }

			void setTainted(bool b) { tainted = b; }
		};

    list<CptAlias *> work_list;
		list<Function *> func_list;
		list<Function *> uncalled_list;
		map<Value *, CptNode *> value_map;
		CptNode *root_node;
		set<CptNode *> freed_nodes;
    map<Instruction *, list<list<Operator *>>> inst_map;

    bool runOnModule(Module &M) override;
		void getAnalysisUsage(AnalysisUsage &AU) const;
    CodePtrAliasPass::CptNode* getRootNode();
    map<Value *, CodePtrAliasPass::CptNode *> getValueMap();
    list<Function *>  getUncalledList();

		LLVMContext *C;
		const DataLayout *DL;

  private:  
		//void getFunctionsFromCallGraph(Module &M);
    void handleLoadPtrOperand(Value *pV);
    void handleLoads(Module &M);
		void handleGlobalVariables(Module &M);
		void handleOperators(GlobalVariable *pV);
    void getUsersOfGV(Value *pV, list<Operator *> op_list);
		void handleGlobalPointers(Module &M);
		void handleInstructions(Module &M);
		void handleFunctions(Module &M);
		void handleArguments(Function *pF);
		void getFunctionsFromCallGraph(Module &M);
		//void handleCmdLineArguments(Module &M);
		void getPtrAliases(CptAlias *alias);
		void mergeNode(CptNode *dst, CptNode *src);
    Value *getArgument(Value *pI, Value *pV);
    bool index_compare(std::list<Value*> idx_list_a, std::list<Value*> idx_list_b, int start_idx);
		void dump();
		void printNode(CptNode *node);
    void replaceGEPOps(Module &M);
    bool IsStructTy(Type *ty);
		void init(Module &M);
  };
//}

