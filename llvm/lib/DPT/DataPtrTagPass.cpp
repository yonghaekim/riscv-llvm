#include "llvm/DPT/DataPtrTagPass.h"
#include "llvm/DPT/DPT.h"

#define PARTS_USE_SHA3

extern "C" {
#include "../PARTS-sha3/include/sha3.h"
}

char DataPtrTagPass::ID = 0;
static RegisterPass<DataPtrTagPass> X("dpt-tag", "Data-pointer tagging pass");

Pass *llvm::DPT::createDataPtrTagPass() { return new DataPtrTagPass(); }

bool DataPtrTagPass::runOnModule(Module &M) {
	init(M);
  auto const dptInstType = llvm::DPT::getDptInstType();
  auto const dptQemuMode = llvm::DPT::getDptQemuMode();

  if (dptInstType == DptType::None) {
		insertDptSet(M);
	  return false;
	}

	DPT_H = (dptInstType == DptType::DPT_H);
	DPT_C = (dptInstType == DptType::DPT_C);
	DPT_F = (dptInstType == DptType::DPT_F);
	QEMU = (dptQemuMode == DptQemuEn::Enable);

	assert(DPT_H || DPT_C || DPT_F);

	errs() << "Start DPT pass!\n";
	errs() << "-- DPT_H: " << DPT_H << "\n";
	errs() << "-- DPT_C: " << DPT_C << "\n";
	errs() << "-- DPT_F: " << DPT_F << "\n";
	errs() << "-- QEMU : " << QEMU << "\n";

	handleIntrinsicFunctions(M);

	if (DPT_C || DPT_F) {
  	handleGlobalVariables(M);
  	handleAllocas(M);
	}
	
  handleMallocs(M);

	if (DPT_F)
		handleGEPs(M);

	// For Juliet test, need to comment out
  handlePtrToInts(M);

	if (QEMU)
		handleQEMU(M);

	insertDptSet(M);

	errs() << "statNumGV: " << statNumGV << "\n";
	errs() << "statNumAI: " << statNumAI << "\n";
	errs() << "statNumCI: " << statNumCI << "\n";
	errs() << "statNumGEP: " << statNumGEP << "\n";

	return true; // function_modified = true
}

void DataPtrTagPass::handleGlobalVariables(Module &M) {
	set<GlobalVariable *> gv_set;

	for (auto &G : M.getGlobalList()) {
		GlobalVariable *pGV = dyn_cast<GlobalVariable>(&G);
		Type *ty = pGV->getType()->getElementType();

		// Skip ".str" constants
		if ((pGV->isConstant() && pGV->getName().find(".str") == 0) ||
				(pGV->use_empty() || pGV->getLinkage() == GlobalValue::LinkOnceODRLinkage) ||
				(!pGV->hasInitializer() || pGV->isDeclaration()) ||
				(!ty->isArrayTy() && !IsStructTyWithArray(ty)))
			continue;

		statNumGV++;
		gv_set.insert(pGV);
  }

	for (auto pGV: gv_set)
		handleGlobalVariable(M, pGV);
}

void DataPtrTagPass::handleGlobalVariable(Module &M, GlobalVariable *pGV) {
	handleOperators(M, pGV); //

	Function *main = NULL;
	for (auto &F : M)
		if (&F && F.getName() == "main")
			main = &F;

	Type *ty = pGV->getType()->getElementType();
	auto size = DL->getTypeSizeInBits(ty);

	// for now, size > 4GB is not tagged
	if (size >= ((size_t) 1 << 32))
		return;

	Value *arg = ConstantInt::get(Type::getInt64Ty(*C), size / 8);
  auto typeIdConstant = getTypeIDConstantFrom(*(pGV->getType()), *C);
	set<Type *> type_set;
	type_set = getStructTypes(ty, type_set);
	map<Function *, set<Instruction *>> func_map;

  // Find all user functions
  for (auto pU: pGV->users()) {
    if (auto *pI = dyn_cast<Instruction>(pU)) {
			if (pI->getFunction()->getSection().find(".text.startup") != std::string::npos)
				continue;

			if (auto pF = pI->getFunction()) {
				func_map[pF].insert(pI);
				func_map[main].insert(pI);
			}
		}
	}

	for (auto &x: func_map) {
		auto pF = x.first;
		auto &BB = pF->front();
		auto &I = BB.front();
		IRBuilder<> Builder(&I);

		// Insert tagd
		FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
		auto tagd = QEMU? M.getOrInsertFunction("__tagd_mod", FuncTypeA) :
											Intrinsic::getDeclaration(&M, Intrinsic::dpt_tagd_mod, {Type::getInt8PtrTy(*C)});
		auto castA = Builder.CreateCast(Instruction::BitCast, pGV, Type::getInt8PtrTy(*C));
		auto callA = Builder.CreateCall(tagd, {castA, typeIdConstant}, ""); 

		if (pF->getName() == "main") {
			// Insert cstr
			auto cstr = QEMU? M.getOrInsertFunction("__cstr", FuncTypeA) :
												Intrinsic::getDeclaration(&M, Intrinsic::dpt_cstr);
			Builder.CreateCall(cstr, {callA, arg}, ""); 
		}

		auto castB = Builder.CreateCast(Instruction::BitCast, callA, pGV->getType());

		for (auto pI: x.second) {
			if (pF != pI->getFunction())
				continue;

			bool chk = false;
			unsigned cnt = 0;
			for (auto it = pI->op_begin(); it != pI->op_end(); it++) {
				if ((*it) == pGV) {
					pI->setOperand(cnt, castB);
					chk = true;
					break;
				}
				cnt++;
			}

			assert(chk);
		}
	}
}

void DataPtrTagPass::handleAllocas(Module &M) {
	set<AllocaInst *> alloca_set;

	for (auto &F : M) {
		for (auto &BB : F) {
			for (auto &I : BB) {
				if (auto pAI = dyn_cast<AllocaInst>(&I)) {
					Type *ty = pAI->getAllocatedType();
					Function *pF = pAI->getFunction();

					if (ty->isArrayTy() || IsStructTyWithArray(ty)) {
					//if (ty->isArrayTy()) {
						alloca_set.insert(pAI);
						statNumAI++;
					//} else if (pAI->isArrayAllocation()) {
					//	// To handle void *alloca(size_t size); 
					//	// Found in Juliet test suite
					//	// This is not in POSIX...
					//	alloca_set.insert(pAI);
					//	statNumAI++;
					//	//errs() << "pAI->dump(): "; pAI->dump();
					//  //auto size = pAI->getAllocationSizeInBits(*DL);
					//	//errs() << "size: " << size.getValue() << "\n";
					}
				}
			}
		}
	}

	for (auto pAI: alloca_set)
		handleAlloca(M, pAI);
}

void DataPtrTagPass::handleAlloca(Module &M, AllocaInst *pAI) {
	Type *ty = pAI->getAllocatedType();
  auto size = pAI->getAllocationSizeInBits(*DL);
	if (size == llvm::None)
		return;

	// for now, size > 4GB is not tagged
	if (size.getValue() >= ((size_t) 1 << 32))
		return;

  // Check Return Inst
	bool chk = false;
	auto pF = pAI->getFunction();
	Instruction *pIB = nullptr;

	for (auto &BB: *pF) {
		for (auto &I: BB) {
			if (dyn_cast<ReturnInst>(&I)) {
				pIB = &I;
				chk = true;
				break;
			}
		}
	}

	if (!chk)
		return;

	if (dyn_cast<UnreachableInst>(pIB))
		return;

	Value *arg = ConstantInt::get(Type::getInt64Ty(*C), (*size) / 8);
  IRBuilder<> Builder(pAI->getNextNode());

	// Insert tagd / cstr
	FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C)}, false);
	FunctionType *FuncTypeB = FunctionType::get(Type::getVoidTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
	auto tagd = QEMU? M.getOrInsertFunction("__tagd", FuncTypeA) :
										Intrinsic::getDeclaration(&M, Intrinsic::dpt_tagd, {Type::getInt8PtrTy(*C)});
	auto castA = dyn_cast<Instruction>(Builder.CreateCast(Instruction::BitCast, pAI, Type::getInt8PtrTy(*C)));
	auto callA = Builder.CreateCall(tagd, {castA}, "");

	auto cstr = QEMU? M.getOrInsertFunction("__cstr", FuncTypeB) :
										Intrinsic::getDeclaration(&M, Intrinsic::dpt_cstr);
	Builder.CreateCall(cstr, {callA, arg}, ""); 
	auto castB = Builder.CreateCast(Instruction::BitCast, callA, pAI->getType());

	pAI->replaceAllUsesWith(castB);
	// alloca() can have int8* type
	// In this case, castA is not created...
	if (pAI->getType() == Type::getInt8PtrTy(*C))
		callA->setOperand(0, pAI);
	else
		castA->setOperand(0, pAI);

	//// For now, supprt for alloca() is limited...
	//// cclr is not inserted
	//if (!ty->isArrayTy() && !IsStructTyWithArray(ty))
	//	return;

	IRBuilder<> BuilderB(pIB);
	FunctionType *FuncTypeC = FunctionType::get(Type::getVoidTy(*C), {Type::getInt8PtrTy(*C)}, false);
	auto cclr = QEMU? M.getOrInsertFunction("__cclr", FuncTypeC) :
										Intrinsic::getDeclaration(&M, Intrinsic::dpt_cclr);
	BuilderB.CreateCall(cclr, {callA});

	if (DPT_F && IsStructTyWithArray(ty)) {
		IRBuilder<> BuilderB(pIB);
		FunctionType *FuncTypeA = FunctionType::get(Type::getVoidTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
		auto scan = M.getOrInsertFunction("__scan_bitmap", FuncTypeA);
		BuilderB.CreateCall(scan, {callA, arg});
	}
}

void DataPtrTagPass::handleMallocs(Module &M) {
	set<Instruction *> malloc_set;
	set<Instruction *> free_set;

	for (auto &F : M) {
		for (auto &BB : F) {
			for (auto &I : BB) {
        switch (I.getOpcode()) {
          case Instruction::Call:
          case Instruction::Invoke:
          {
						Function *pF = nullptr;

						if (CallInst *pCI = dyn_cast<CallInst>(&I))
            	pF = pCI->getCalledFunction();
						else if (InvokeInst *pII = dyn_cast<InvokeInst>(&I))
            	pF = pII->getCalledFunction();

            if (pF && (pF->getName() == "malloc" ||
                        pF->getName() == "_Znwm" /* new */ ||
                        pF->getName() == "_Znam" /* new[] */ ||
                        pF->getName() == "calloc" ||
                        pF->getName() == "realloc")) {
							statNumCI++;
              malloc_set.insert(&I);
            } else if (pF && (pF->getName() == "free" ||
															pF->getName() == "_ZdlPv" ||
															pF->getName() == "_ZdaPv")) {
              free_set.insert(&I);
            }
					}
					default:
						break;
				}
			}
		}
	}

	for (auto pI: malloc_set)
		handleMalloc(M, pI);

	for (auto pI: free_set)
		handleFree(M, pI);
}

void DataPtrTagPass::handleMalloc(Module &M, Instruction *pI) {
	Function *called_func = nullptr;

	if (CallInst *pCI = dyn_cast<CallInst>(pI)) {
		called_func = pCI->getCalledFunction();
		auto arg0 = pCI->getArgOperand(0);

		if (called_func && (called_func->getName() == "malloc" ||
				called_func->getName() == "_Znwm" /* new */ ||
				called_func->getName() == "_Znam" /* new[] */)) {

			IRBuilder<> Builder(pCI->getNextNode());
  		FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C)}, false);
  		FunctionType *FuncTypeB = FunctionType::get(Type::getVoidTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
  		auto tag = QEMU? M.getOrInsertFunction("__tagd", FuncTypeA) :
											Intrinsic::getDeclaration(&M, Intrinsic::dpt_tagd, {Type::getInt8PtrTy(*C)});
			auto callA = Builder.CreateCall(tag, {pCI}, "");
			auto cstr = QEMU? M.getOrInsertFunction("__cstr", FuncTypeB) : 
												Intrinsic::getDeclaration(&M, Intrinsic::dpt_cstr);
			Builder.CreateCall(cstr, {callA, arg0}, ""); 
			pCI->replaceAllUsesWith(callA);
			callA->setOperand(0, pCI);
		} else if (called_func && called_func->getName() == "calloc") {
			auto arg1 = pCI->getArgOperand(1);

			IRBuilder<> Builder_prev(pCI);
			Value *res = Builder_prev.CreateMul(arg0, arg1);

			IRBuilder<> Builder(pCI->getNextNode());
  		FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C)}, false);
  		FunctionType *FuncTypeB = FunctionType::get(Type::getVoidTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
  		auto tag = QEMU? M.getOrInsertFunction("__tagd", FuncTypeA) :
											Intrinsic::getDeclaration(&M, Intrinsic::dpt_tagd, {Type::getInt8PtrTy(*C)});
			auto callA = Builder.CreateCall(tag, {pCI}, "");
			auto cstr = QEMU? M.getOrInsertFunction("__cstr", FuncTypeB) :
												Intrinsic::getDeclaration(&M, Intrinsic::dpt_cstr);
			Builder.CreateCall(cstr, {callA, res}, ""); 
			pCI->replaceAllUsesWith(callA);
			callA->setOperand(0, pCI);
		} else if (called_func && called_func->getName() == "realloc") {
			auto arg1 = pCI->getArgOperand(1);

			IRBuilder<> BuilderA(pCI);
			FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C)}, false);
			auto handle = M.getOrInsertFunction("__xtag_ptr_heap", FuncTypeA);
			auto call = BuilderA.CreateCall(handle, {arg0});
			pCI->setOperand(0, call);

			IRBuilder<> BuilderB(pCI->getNextNode());
			FunctionType *FuncTypeB = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
			auto handleB = M.getOrInsertFunction("__tag_ptr", FuncTypeB);
			auto callB = BuilderB.CreateCall(handleB, {pCI, arg1});
			pCI->replaceAllUsesWith(callB);
			callB->setOperand(0, pCI);
		}
	} else if (InvokeInst *pII = dyn_cast<InvokeInst>(pI)) {
		called_func = pII->getCalledFunction();
		auto arg0 = pII->getArgOperand(0);

		if (called_func && (called_func->getName() == "malloc" ||
				called_func->getName() == "_Znwm" /* new */ ||
				called_func->getName() == "_Znam" /* new[] */)) {

      auto pBB = pII->getNormalDest();
			auto pFI = &(pBB->front());

			if (PHINode *pPN = dyn_cast<PHINode>(pFI)) {
				// TODO
			} else {
				IRBuilder<> Builder(&(pBB->front()));
				FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C)}, false);
				FunctionType *FuncTypeB = FunctionType::get(Type::getVoidTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
				auto tag = QEMU? M.getOrInsertFunction("__tagd", FuncTypeA) :
												Intrinsic::getDeclaration(&M, Intrinsic::dpt_tagd, {Type::getInt8PtrTy(*C)});
				auto callA = Builder.CreateCall(tag, {pII}, "");
				auto cstr = QEMU? M.getOrInsertFunction("__cstr", FuncTypeB) : 
													Intrinsic::getDeclaration(&M, Intrinsic::dpt_cstr);
				Builder.CreateCall(cstr, {callA, arg0}, ""); 

				pII->replaceAllUsesWith(callA);
				callA->setOperand(0, pII);
			}
		} else if (called_func && called_func->getName() == "calloc") {
			auto arg1 = pII->getArgOperand(1);

			IRBuilder<> Builder_prev(pII);
			Value *res = Builder_prev.CreateMul(arg0, arg1);

			IRBuilder<> Builder(pCI->getNextNode());
  		FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C)}, false);
  		FunctionType *FuncTypeB = FunctionType::get(Type::getVoidTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
  		auto tag = QEMU? M.getOrInsertFunction("__tagd", FuncTypeA) :
											Intrinsic::getDeclaration(&M, Intrinsic::dpt_tagd, {Type::getInt8PtrTy(*C)});
			auto callA = Builder.CreateCall(tag, {pII}, "");
			auto cstr = QEMU? M.getOrInsertFunction("__cstr", FuncTypeB) :
											Intrinsic::getDeclaration(&M, Intrinsic::dpt_cstr);
			Builder.CreateCall(cstr, {callA, res}, ""); 
			pII->replaceAllUsesWith(callA);
			callA->setOperand(0, pII);
		} else if (called_func && called_func->getName() == "realloc") {
			auto arg1 = pII->getArgOperand(1);

			IRBuilder<> BuilderA(pII);
			FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C)}, false);
			auto handle = M.getOrInsertFunction("__xtag_ptr_heap", FuncTypeA);
			auto call = BuilderA.CreateCall(handle, {arg0});
			pII->setOperand(0, call);

			IRBuilder<> BuilderB(pII->getNextNode());
			FunctionType *FuncTypeB = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
			auto handleB = M.getOrInsertFunction("__tag_ptr", FuncTypeB);
			auto callB = BuilderB.CreateCall(handleB, {pII, arg1});
			pII->replaceAllUsesWith(callB);
			callB->setOperand(0, pII);
		}
	}
}

void DataPtrTagPass::handleFree(Module &M, Instruction *pI) {
	Value *arg = pI->getOperand(0);

	IRBuilder<> Builder(pI);
	Type *numType = Type::getInt64Ty(*C);

	FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C)}, false);
	FunctionType *FuncTypeB = FunctionType::get(Type::getVoidTy(*C), {Type::getInt8PtrTy(*C)}, false);
	auto cclr = QEMU ? M.getOrInsertFunction("__cclr", FuncTypeB) :
								Intrinsic::getDeclaration(&M, Intrinsic::dpt_cclr);
	Builder.CreateCall(cclr, {arg});
	auto xtag = QEMU? M.getOrInsertFunction("__xtag", FuncTypeA) :
										Intrinsic::getDeclaration(&M, Intrinsic::dpt_xtag, {Type::getInt8PtrTy(*C)});
	auto callA = Builder.CreateCall(xtag, {arg});
	if (DPT_F) {
		auto scan = M.getOrInsertFunction("__scan_bitmap_heap", FuncTypeB);
		Builder.CreateCall(scan, {arg});
	}
	pI->setOperand(0, callA);
}

void DataPtrTagPass::handleGEPs(Module &M) {
	list <GetElementPtrInst *> gep_list;

	for (auto &F : M) {
		if (F.getSection().find(".text.startup") != std::string::npos)
			continue;

		for (auto &BB: F) {
			for (auto &I: BB) {
				if (auto pGEP = dyn_cast<GetElementPtrInst>(&I)) {
					// If ResultElementType is array type
					if (pGEP->getResultElementType()->isArrayTy()) {
						// Do not narrow array-to-array GEPs
						// [TODO] Do not narrow struct array-to-struct GEPs
						if (!pGEP->getSourceElementType()->isArrayTy() &&
								!pGEP->getResultElementType()->getArrayElementType()->isStructTy())
							gep_list.push_back(pGEP);

						//if (pGEP->getResultElementType()->getArrayElementType()->isStructTy()) {
						//	errs() << "GEP: "; pGEP->dump();
						//	errs() << "Type: "; pGEP->getResultElementType()->dump();
						//}
					}
				}
			}
		}
	}

	for (auto pGEP: gep_list)
		handleGEP(M, pGEP);
	for (auto pGEP: gep_erase_set)
		pGEP->eraseFromParent();
}

void DataPtrTagPass::handleGEP(Module &M, GetElementPtrInst *pGEP) {
	// Skip if numIndices == 1 && index == 0
	if (pGEP->getNumIndices() == 1) {
		ConstantInt *idx = dyn_cast<ConstantInt>(*(pGEP->idx_begin()));

		if (idx && idx->getSExtValue() == 0)
			return;
	}

	auto size = DL->getTypeSizeInBits(pGEP->getResultElementType());

	// Workaround the issue with custom data structure (e.g., arr[0])
	if (size == 0)
		return;

	// Check if this GEP has already been handled
	if (gep_visit_set.find(pGEP) != gep_visit_set.end())
		return;

	Value *arg = ConstantInt::get(Type::getInt64Ty(*C), size / 8);
	auto base = pGEP->getPointerOperand();

	//if (auto pGV = dyn_cast<GlobalVariable>(base)) {
	//	errs() << "Found GV base: "; pGV->dump();
	//}
	list <GetElementPtrInst *> gep_list;
	gep_list.push_back(pGEP);
	gep_visit_set.insert(pGEP);

	// Find GEPs that are accessed from the same base with the same indices
	// Replace all such GEPs with "a" mutated pointer
	//errs() << "base->dump(): "; base->dump();
	for (auto pU: base->users()) {
		if (pU == pGEP)
			continue;

		if (auto gep_user = dyn_cast<GetElementPtrInst>(pU)) {
			// If # indices are different, no need to compare indices
			if (pGEP->getNumIndices() != gep_user->getNumIndices())
				continue;

			// If not in the same BB, shouldn't touch
			if (pGEP->getParent() != gep_user->getParent())
				continue;

			int i = 0;
			for (i=0; i<(pGEP->getNumIndices() + 1); i++) {
				if (pGEP->getOperand(i) != gep_user->getOperand(i))
					break;
			}

			if (i == (pGEP->getNumIndices() + 1)) {
				//errs() << "gep_user->dump(): "; gep_user->dump();
				gep_list.push_back(gep_user);
				gep_visit_set.insert(gep_user);
			}
		}	
	}

	IRBuilder<> Builder(pGEP->getNextNode());
	FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
	FunctionType *FuncTypeB = FunctionType::get(Type::getVoidTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
	auto typeIdConstant = getTypeIDConstantFrom(*(Type::getInt64Ty(*C)), *C);
	Value *zero = ConstantInt::get(Type::getInt64Ty(*C), 0); // temp
	auto tagd = QEMU? M.getOrInsertFunction("__tagd_mod", FuncTypeA) :
										Intrinsic::getDeclaration(&M, Intrinsic::dpt_tagd_mod, {Type::getInt8PtrTy(*C)});
	auto castA = dyn_cast<Instruction>(Builder.CreateCast(Instruction::BitCast, pGEP, Type::getInt8PtrTy(*C)));
	auto callA = Builder.CreateCall(tagd, {castA, zero}, ""); 
	auto mutate = M.getOrInsertFunction("__mutate_ptr", FuncTypeB);
	auto callB = Builder.CreateCall(mutate, {callA, arg});
	if (auto pCI = dyn_cast<CallInst>(callB)) {
		auto func = pCI->getCalledFunction();
		func->addFnAttr(Attribute::AlwaysInline);
		func->removeFnAttr(Attribute::NoInline);
		func->removeFnAttr(Attribute::OptimizeNone);
	}
	auto castB = Builder.CreateCast(Instruction::BitCast, callA, pGEP->getType());

	for (auto gep: gep_list) {
		gep->replaceAllUsesWith(castB);
		if (gep != pGEP)
			gep_erase_set.insert(gep);
	}

	castA->setOperand(0, pGEP);
	statNumGEP++;
}

void DataPtrTagPass::handleOperators(Module &M, GlobalVariable *pGV) {
	map<Operator *, set<Instruction *>> op_map;
	Value *arg = ConstantInt::get(Type::getInt64Ty(*C), 0); // temp

  // Find users of operators of GVs
  for (auto pU: pGV->users()) {
    if (dyn_cast<Instruction>(pU)) {
			// skip
    } else if (auto *pOp = dyn_cast<Operator>(pU)) {
	    for (auto pUb: pOp->users()) {
		    if (auto *pI = dyn_cast<Instruction>(pUb)) {
					if (dyn_cast<PHINode>(pI))
						continue;

					op_map[pOp].insert(pI);
				} else if (dyn_cast<Operator>(pUb))
					assert(false);
			}
		}
	}

	for (auto &x: op_map) {
		auto pOp = x.first;

		for (auto pI: x.second) {
			IRBuilder<> Builder(pI);
			Instruction *new_inst = NULL;

			auto tagd = Intrinsic::getDeclaration(&M, Intrinsic::dpt_tagd, {Type::getInt8PtrTy(*C)});
			auto castA = Builder.CreateCast(Instruction::BitCast, pGV, Type::getInt8PtrTy(*C));
			auto callA = Builder.CreateCall(tagd, {castA, arg}, ""); 
			auto castB = dyn_cast<Instruction>(Builder.CreateCast(Instruction::BitCast, callA, pGV->getType()));
			assert(castB);

			if (auto *pGEP = dyn_cast<GEPOperator>(pOp)) {
				vector<Value *> indices;
				for (auto idx = pGEP->idx_begin(); idx != pGEP->idx_end(); idx++)
					indices.push_back(*idx);

				new_inst = dyn_cast<Instruction>(Builder.CreateGEP(castB, indices));
				new_inst->setOperand(0, pGV);
			} else if (auto *pBC = dyn_cast<BitCastOperator>(pOp)) {
				new_inst = dyn_cast<Instruction>(Builder.CreateCast(Instruction::BitCast, castB, pBC->getType()));
				new_inst->setOperand(0, pGV);
			} else if (auto *pPTI = dyn_cast<PtrToIntOperator>(pOp)) {
				new_inst = dyn_cast<Instruction>(Builder.CreateCast(Instruction::PtrToInt, castB, pPTI->getType()));
				new_inst->setOperand(0, pGV);
			} else {
				errs() << "Found non-defined operator\n";
				assert(false);
			}

			bool chk = false;
			unsigned cnt = 0;
			for (auto it = pI->op_begin(); it != pI->op_end(); it++) {
				if ((*it) == pOp) {
					pI->setOperand(cnt, new_inst);
					chk = true;
					break;
				}
				cnt++;
			}

			assert(chk);
			castB->eraseFromParent();
			callA->eraseFromParent();
		}
	}
}

void DataPtrTagPass::handlePtrToInts(Module &M) {
	for (auto &F : M) {
		for (auto &BB : F) {
			for (auto &I : BB) {
				if (auto pPTI = dyn_cast<PtrToIntInst>(&I)) {
					addXtag(pPTI, 0, M);
				} else if (auto pCI = dyn_cast<ICmpInst>(&I)) {
					//pCI->dump();
					assert(pCI->getNumOperands() == 2);

					Value *val0 = pCI->getOperand(0);
					auto *const0 = dyn_cast<ConstantPointerNull>(val0);
					Type *ty0 = pCI->getOperand(0)->getType();

					if (PointerType *pty = dyn_cast<PointerType>(ty0)) {
						if (!const0)
							addXtag(pCI, 0, M);
					}

					Value *val1 = pCI->getOperand(1);
					auto *const1 = dyn_cast<ConstantPointerNull>(val1);
					Type *ty1 = pCI->getOperand(1)->getType();

					if (PointerType *pty = dyn_cast<PointerType>(ty1)) {
						if (!const1)
							addXtag(pCI, 1, M);
					}
        }
			}
		}
	}
}

void DataPtrTagPass::addXtag(Instruction *pI, unsigned idx, Module &M) {
  auto ptr = pI->getOperand(idx);

  IRBuilder<> Builder(pI);
	//Value *num = ConstantInt::get(Type::getInt64Ty(*C), intr_num++);
  auto castA = Builder.CreateCast(Instruction::BitCast, ptr, Type::getInt8PtrTy(*C));
  FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C)}, false);
  auto xtag = QEMU? M.getOrInsertFunction("__xtag", FuncTypeA) :
										Intrinsic::getDeclaration(&M, Intrinsic::dpt_xtag, {Type::getInt8PtrTy(*C)});
  auto callA = Builder.CreateCall(xtag, {castA}, "");
  //FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
  //auto xtag = M.getOrInsertFunction("__xtag_num", FuncTypeA);
  //auto callA = Builder.CreateCall(xtag, {castA, num}, "");
  auto castB = Builder.CreateCast(Instruction::BitCast, callA, ptr->getType());

  pI->setOperand(idx, castB);
}

bool DataPtrTagPass::IsStructTyWithArray(Type *ty) {
	while (ty->isArrayTy())
		ty = ty->getArrayElementType();

	if (auto str_ty = dyn_cast<StructType>(ty)) {
		for (auto it = str_ty->element_begin(); it != str_ty->element_end(); it++) {
			Type *ety = (*it);

			if (ety->isArrayTy())
				return true;
			if (ety->isStructTy() && IsStructTyWithArray(ety))
				return true;
		}
	}

	return false;
}

set<Type *> DataPtrTagPass::getStructTypes(Type *ty, set<Type *> type_set) {
	while (ty->isArrayTy())
		ty = ty->getArrayElementType();

	if (auto str_ty = dyn_cast<StructType>(ty)) {
		for (auto it = str_ty->element_begin(); it != str_ty->element_end(); it++) {
			Type *ety = (*it);

			if (ety->isArrayTy())
				type_set.insert(str_ty);

			//if (ety->isStructTy())
			if (IsStructTyWithArray(ety))
				type_set = getStructTypes(ety, type_set);
		}
	}

	return type_set;
}

bool DataPtrTagPass::IsStructTy(Type *ty) {
	if (ty->isStructTy())
		return true;
	else if (!ty->isArrayTy())
		return false;

	while (ty->isArrayTy()) {
		ty = ty->getArrayElementType();

		if (ty->isStructTy())
			return true;
	}

	return false;
}

void DataPtrTagPass::init(Module &M) {
	for (auto &F : M) {
		if (&F && F.getName() == "main") {
		  C = &F.getContext();
			DL = &F.getParent()->getDataLayout();

			return;
		}
	}

	for (auto &F : M) {
		if (&F) {
		  C = &F.getContext();
			DL = &F.getParent()->getDataLayout();

			return;
		}
	}
}

// auto typeIdConstant = PARTS::getTypeIDConstantFrom(*calledValueType, F.getContext());
// auto paced = Builder.CreateCall(autcall, { calledValue, typeIdConstant }, "");

void DataPtrTagPass::buildTypeString(const Type *T, llvm::raw_string_ostream &O) {
  if (T->isPointerTy()) {
    O << "ptr.";
    buildTypeString(T->getPointerElementType(), O);
  } else if (T->isStructTy()) {
		auto sty = dyn_cast<StructType>(T);
		std::regex e("^(\\w+\\.\\w+)(\\.\\w+)?$");

		if (sty->isLiteral()) {
			O << std::regex_replace("str.", e, "$1");
		} else {
			auto structName = dyn_cast<StructType>(T)->getStructName();
			O << std::regex_replace(structName.str(), e, "$1");
		}
  } else if (T->isArrayTy()) {
    O << "arr.";
    buildTypeString(T->getArrayElementType(), O);
  } else if (T->isFunctionTy()) {
    auto FuncTy = dyn_cast<FunctionType>(T);
    O << "f.";
    buildTypeString(FuncTy->getReturnType(), O);

    for (auto p = FuncTy->param_begin(); p != FuncTy->param_end(); p++) {
      buildTypeString(*p, O);
    }
  } else if (T->isVectorTy()) {
    O << "vec." << T->getVectorNumElements();
    buildTypeString(T->getVectorElementType(), O);
  } else if (T->isVoidTy()) {
    O << "v";
  } else {
    /* Make sure we've handled all cases we want to */
    assert(T->isIntegerTy() || T->isFloatingPointTy());
    T->print(O);
  }
}


uint64_t DataPtrTagPass::getTypeIDFor(const Type *T) {
  if (!T->isPointerTy())
    return 0; // Not a pointer, hence no type ID for this one

  // TODO: This should perform caching, so calling the same Type will not
  // reprocess the stuff. Use a Dictionary-like ADT is suggested.
  decltype(TypeIDCache)::iterator id;
  if ((id = TypeIDCache.find(T)) != TypeIDCache.end())
    return id->second;

  uint64_t theTypeID = 0;
  std::string buf;
  llvm::raw_string_ostream typeIdStr(buf);

  buildTypeString(T, typeIdStr);
  typeIdStr.flush();

  // Prepare SHA3 generation
  auto rawBuf = buf.c_str();
  mbedtls_sha3_context sha3_context;
  mbedtls_sha3_type_t sha3_type = MBEDTLS_SHA3_256;
  mbedtls_sha3_init(&sha3_context);

  // Prepare input and output variables
  auto *input = reinterpret_cast<const unsigned char *>(rawBuf);
  auto *output = new unsigned char[32]();

  // Generate hash
  auto result = mbedtls_sha3(input, buf.length(), sha3_type, output);
  if (result != 0)
    llvm_unreachable("SHA3 hashing failed :(");
  memcpy(&theTypeID, output, sizeof(theTypeID));
  // TODO need to fix delete[] output;

  TypeIDCache.emplace(T, theTypeID);

  return theTypeID;
}

Constant *DataPtrTagPass::getTypeIDConstantFrom(const Type &T, LLVMContext &C) {
  return Constant::getIntegerValue(Type::getInt64Ty(C),
                                   //APInt(64, getTypeIDFor(&T) & 0xFFFF));
                                   APInt(64, getTypeIDFor(&T)));

  //return Constant::getIntegerValue(Type::getInt64Ty(C),
  //                                 APInt(64, 0));
}

void DataPtrTagPass::handleIntrinsicFunctions(Module &M) {
	for (auto &F: M) {
    for (auto &BB: F) {
      for (auto &I: BB) {
				Function *pF = nullptr;

        if (CallInst *pCI = dyn_cast<CallInst>(&I))
          pF = pCI->getCalledFunction();
        else if (InvokeInst *pII = dyn_cast<InvokeInst>(&I))
          pF = pII->getCalledFunction();

				// For Juliet test, need to comment out
				// Handle calls using function pointers
				if (!pF)
					handleIntrinsicFunction(M, &I);

				if (pF && pF->isDeclaration()) {
					//if (pF->getName() == "malloc" ||
					//		pF->getName() == "_Znwm" ||
					//		pF->getName() == "_Znam" ||
					//		pF->getName() == "calloc" ||
					//		pF->getName() == "realloc" ||
					//		pF->getName() == "free" ||
					//		pF->getName() == "_ZdlPv" ||
					//		pF->getName() == "_ZdaPv")
					//	continue;

					if (pF->getName() == "strlen" ||
							pF->getName() == "strchr" ||
							pF->getName() == "strtol" ||
							pF->getName() == "strcmp" ||
							pF->getName() == "strcpy" ||
							pF->getName() == "strncpy" ||
							pF->getName() == "strncmp" ||
							pF->getName() == "strstr" ||
							pF->getName() == "strspn" ||
							pF->getName() == "strcspn" ||
							pF->getName() == "strrchr" ||
							pF->getName() == "strtoul" ||
							pF->getName() == "strtod" ||
							pF->getName() == "strcat" ||
							pF->getName() == "strncat" ||
							pF->getName() == "strcasecmp" ||
							pF->getName() == "strncasecmp" ||
							pF->getName() == "atoi" ||
							pF->getName() == "atof" ||
							pF->getName() == "atol" ||
							//pF->getName() == "read" ||
							pF->getName() == "llvm.memcpy.p0i8.p0i8.i64" ||
							pF->getName() == "llvm.memset.p0i8.i64" ||
							pF->getName() == "llvm.memmove.p0i8.p0i8.i64" ||
							pF->getName() == "llvm.lifetime.start.p0i8" ||
							pF->getName() == "llvm.lifetime.end.p0i8" ||
							pF->getName() == "llvm.va_start" ||
							pF->getName() == "llvm.va_end" ||
							pF->getName() == "__isoc99_sscanf" ||
							pF->getName() == "malloc" ||
							pF->getName() == "_Znwm" ||
							pF->getName() == "_Znam" ||
							pF->getName() == "calloc" ||
							pF->getName() == "realloc" ||
							pF->getName() == "free" ||
							pF->getName() == "_ZdlPv" ||
							pF->getName() == "_ZdaPv" ||
							pF->getName() == "memcmp" ||
							//pF->getName() == "getc" ||
							//pF->getName() == "putc" ||
							pF->getName() == "printf" ||
							pF->getName() == "scanf" ||
							pF->getName() == "sprintf" ||
							//pF->getName() == "fgetc" ||
							//pF->getName() == "fprintf" ||
							//pF->getName() == "fopen" ||
							//pF->getName() == "fclose" ||
							//pF->getName() == "ferror" ||
							//pF->getName() == "fgets" ||
							//pF->getName() == "fread" ||
							//pF->getName() == "fwrite" ||
							//pF->getName() == "ftell" ||
							//pF->getName() == "fgets" ||
							//pF->getName() == "fputs" ||
							//pF->getName() == "open" ||
							//pF->getName() == "read" ||
							//pF->getName() == "write" ||
							pF->getName() == "llvm.dbg.declare")
						continue;

					handleIntrinsicFunction(M, &I);
        }
      }
    }
  }
}

void DataPtrTagPass::handleIntrinsicFunction(Module &M, Instruction *pI) {
	bool chk = false;

	if (CallInst *pCI = dyn_cast<CallInst>(pI)) {
		unsigned idx = 0;

		for (auto arg = pCI->arg_begin(); arg != pCI->arg_end(); ++arg) {
			if (Value *pV = dyn_cast<Value>(arg)) {
				if (dyn_cast<ConstantPointerNull>(pV)) {
					idx++;
					continue;
				}

				if (PointerType *pty = dyn_cast<PointerType>(pV->getType())) {
					addXtag(pCI, idx, M);
					chk = true;
				}
			}

			idx++;
		}
	} else if (InvokeInst *pCI = dyn_cast<InvokeInst>(pI)) {
		unsigned idx = 0;

		for (auto arg = pCI->arg_begin(); arg != pCI->arg_end(); ++arg) {
			if (Value *pV = dyn_cast<Value>(arg)) {
				if (dyn_cast<ConstantPointerNull>(pV)) {
					idx++;
					continue;
				}

				if (PointerType *pty = dyn_cast<PointerType>(pV->getType())) {
					addXtag(pCI, idx, M);
					chk = true;
				}
			}

			idx++;
		}
	}

		//if (chk) {
		//	if (pF->getName() == "__cxa_throw" ||
		//			pF->getName() == "__cxa_free_exception" ||
		//			pF->getName() == "__cxa_begin_catch" ||
		//			pF->getName() == "llvm.eh.typeid.for" ||
		//			pF->getName() == "__cxa_atexit") {}
		//	else { errs() << "pII->dump(): "; pII->dump(); }
		//}
	//if (!pF->isDeclaration() && pF->isVarArg())
	//  errs() << "F.getName(): " << pF->getName() << "\n";

}

void DataPtrTagPass::handleQEMU(Module &M) {
	for (auto &F: M) {
    for (auto &BB: F) {
      for (auto &I: BB) {
				bool found = false;
				Value *ptr = nullptr;

        if (LoadInst *pLI = dyn_cast<LoadInst>(&I)) {
					found = true;
					ptr = pLI->getOperand(0);
        } else if (StoreInst *pSI = dyn_cast<StoreInst>(&I)) {
					found = true;
					ptr = pSI->getOperand(1);
				}

				if (found) {
					IRBuilder<> Builder(&I);
					auto castA = Builder.CreateCast(Instruction::BitCast, ptr, Type::getInt8PtrTy(*C));
					//FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C)}, false);
					//auto check = M.getOrInsertFunction("__check", FuncTypeA);
					//auto callA = Builder.CreateCall(check, {castA}, "");
					Value *num = ConstantInt::get(Type::getInt64Ty(*C), intr_num++);
					FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
					auto check = M.getOrInsertFunction("__check_num", FuncTypeA);
					auto callA = Builder.CreateCall(check, {castA, num}, "");
				}
			}
		}
	}
}

//void DataPtrTagPass::handleQEMU(Module &M) {
//	set<string> black_funcs({
//		"fopen",
//		"fopen64",
//		"fclose",
//		"ferror",
//		"fgets",
//		"fgetc",
//		"fputs",
//		"fputc",
//		"fread",
//		"fwrite",
//		"ftell",
//		"fileno",
//		"fdopen",
//		"fstat",
//		"fdopen",
//		"open",
//		"open64",
//		"frexp",
//		"read",
//		"write",
//		"fseek",
//		"fstat64",
//		"lseek",
//		"opendir",
//		"closedir",
//		"readdir",
//		"getenv",
//		"mkdir",
//		"rmdir",
//		"freopen64",
//		"feof",
//		"close",
//		//
//		"__sigsetjmp",
//		"siglongjmp",
//		"chdir",
//		"getcwd",
//		"rename",
//		"access",
//		"link",
//		"unlink",
//		"chmod",
//		"dup",
//		"strerror",
//		"execl",
//		"signal",
//		"time",
//		"localtime",
//		"strftime",
//		"asctime",
//		"localtime_r",
//		"gmtime_r",
//		"gettimeofday",
//		"_exit",
//		"fflush",
//		"select",
//		"__sysv_signal",
//		"pipe",
//		"waitpid",
//		"putenv",
//		"execv",
//		"getenv",
//		"ioctl",
//		"truncate64",
//		"execvp",
//		"execl",
//		"chdir",
//		"getcwd",
//		"rename",
//		"access",
//		"unlink",
//		"chmod",
//		"strerror",
//		"signal",
//		"setvbuf",
//		"clearerr",
//		"stat",
//		"stat64"
//	});
//
//	set<string> white_funcs({
//		"__tagd",
//		"__tagd_mod",
//		"__xtag",
//		"__tag_ptr",
//		"__mutate_ptr",
//		"__xtag_ptr",
//		"__xtag_realloc",
//		"__tag_realloc",
//		"__cstr",
//		"__cclr",
//		"malloc",
//		"_Znwm",
//		"_Znam",
//		"calloc",
//		"realloc",
//		"free",
//		"_ZdlPv",
//		"_ZdaPv",
//		"llvm.dpt.tagd.p0i8",
//		"llvm.dpt.tagd.mod.p0i8",
//		"llvm.dpt.xtag.p0i8",
//		"llvm.dpt.cstr",
//		"llvm.dpt.cclr",
//		// LLVM intrinsics
//		"llvm.memcpy.p0i8.p0i8.i64",
//		"llvm.memset.p0i8.i64",
//		"llvm.memmove.p0i8.p0i8.i64",
//		"llvm.va_start",
//		"llvm.va_end",
//		"llvm.lifetime.start.p0i8",
//		"llvm.lifetime.end.p0i8",
//		"llvm.dbg.declare",
//		"llvm.dbg.label",
//		"llvm.trap",
//		"llvm.round.f64",
//		"llvm.fabs.f64",
//		"llvm.eh.typeid.for",
//		"llvm.floor.f64",
//		"llvm.umul.with.overflow.i64",
//		"llvm.ceil.f64",
//		// I/O functions
//		"__isoc99_sscanf",
//		"scanf",
//		"printf",
//		"sprintf",
//		"getc",
//		"ungetc",
//		"putc",
//		"__isoc99_fscanf",
//		"vprintf",
//		"vfprintf",
//		//"fprintf",
//		//"fopen",
//		//"fopen64",
//		//"fclose",
//		//"ferror",
//		//"fgets",
//		//"fgetc",
//		//"fputs",
//		//"fputc",
//		//"fread",
//		//"fwrite",
//		//"ftell",
//		//"fileno",
//		//"fdopen",
//		//"fstat",
//		//"fdopen",
//		//"open",
//		//"read",
//		//"write",
//		//"fseek",
//		//"lseek",
//		"vsnprintf",
//		"snprintf",
//		// string functions
//		"strlen",
//		"strchr",
//		"strtol",
//		"strcmp",
//		"strcpy",
//		"strncpy",
//		"strncmp",
//		"strstr",
//		"strspn",
//		"strcspn",
//		"strrchr",
//		"strtoul",
//		"strtod",
//		"strcat",
//		"strncat",
//		"strcasecmp",
//		"strncasecmp",
//		"strtok",
//		"strpbrk",
//		"tolower",
//		"toupper",
//		"perror",
//		////
//		"memcmp",
//		"memchr",
//		"chdir",
//		"getcwd",
//		"rename",
//		"access",
//		"unlink",
//		"chmod",
//		"strerror",
//		"execl",
//		"signal",
//		"time",
//		"localtime",
//		"strftime",
//		"asctime",
//		"localtime_r",
//		"gmtime_r",
//		"_exit",
//		"fflush",
//		"qsort",
//		"rand",
//		"srand",
//		"acos",
//		"asin",
//		"atan",
//		"atan2",
//		"sqrt",
//		"sin",
//		"cos",
//		"tan",
//		"exp",
//		"log",
//		"pow",
//		"log10",
//		"tanh",
//		"stat",
//		"fmod",
//		"powf",
//		"__cxa_begin_catch",
//		"__ctype_b_loc",
//		"__cxa_atexit",
//		"__cxa_allocate_exception",
//		"__cxa_throw",
//		"__cxa_free_exception",
//		"__cxa_end_catch",
//		"feof",
//		"close",
//		"gettimeofday",
//		"__errno_location",
//		" __ctype_b_loc",
//		"llvm.prefetch",
//		"__assert_fail",
//		"atoi",
//		"atof",
//		"atol",
//		"exit",
//		"isalpha",
//		"isdigit",
//		"setbuf"
//	});
//
//	for (auto &F: M) {
//    for (auto &BB: F) {
//      for (auto &I: BB) {
//        //if (LoadInst *pLI = dyn_cast<LoadInst>(&I)) {
//        //  addXtag(&I, 0, M);
//        //} else if (StoreInst *pSI = dyn_cast<StoreInst>(&I)) {
//        //  addXtag(&I, 1, M);
//				//} else if (CallInst *pCI = dyn_cast<CallInst>(&I)) {
//				if (CallInst *pCI = dyn_cast<CallInst>(&I)) {
//          Function *pF = pCI->getCalledFunction();
//
//					// Handle calls using function pointers
//					//if (!pF) {
//					//	//errs() << "(1) pCI->dump(): "; pCI->dump();
//          //  bool chk = false;
//          //  unsigned idx = 0;
//      		//	for (auto arg = pCI->arg_begin(); arg != pCI->arg_end(); ++arg) {
//					//		if (Value *pV = dyn_cast<Value>(arg)) {
//          //      if (PointerType *pty = dyn_cast<PointerType>(pV->getType())) {
//          //        addXtag(pCI, idx, M);
//          //        chk = true;
//          //      }
//					//		}
//
//          //    idx++;
//          //  }
//					//}
//
//          //if (pF && (pF->isDeclaration() || pF->isVarArg())) {
//          if (pF && pF->isDeclaration()) {
//						bool match = false;
//						for (auto x: white_funcs) {
//							if (pF->getName() == x) {
//								match = true;
//								//errs() << "pF->getName(): " << pF->getName() << "\n";
//								break;
//							}
//						}
//						if (match)
//							continue;
//
//						//for (auto x: black_funcs) {
//						//	if (pF->getName() == x) {
//						//		match = true;
//						//		break;
//						//	}
//						//}
//						//if (!match)
//						//	continue;
//
//						//errs() << "(2) pCI->dump(): "; pCI->dump();
//            bool chk = false;
//            unsigned idx = 0;
//      			for (auto arg = pCI->arg_begin(); arg != pCI->arg_end(); ++arg) {
//							if (Value *pV = dyn_cast<Value>(arg)) {
//                if (PointerType *pty = dyn_cast<PointerType>(pV->getType())) {
//                  addXtag(pCI, idx, M);
//                  chk = true;
//                }
//							}
//
//              idx++;
//            }
//
//						if (chk) {
//							bool chk2 = false;
//							for (auto x: black_funcs) {
//								if (pF->getName() == x) {
//									chk2 = true;
//									break;
//								}
//							}
//							//if (!chk2)
//							//	errs() << "pF->getName(): " << pF->getName() << "\n";
//						} else {
//							//errs() << "pF->getName(): " << pF->getName() << "\n";
//							assert(false);
//						}
//
//            //if (!pF->isDeclaration() && pF->isVarArg())
//            //  errs() << "F.getName(): " << pF->getName() << "\n";
//          }
//        } else if (InvokeInst *pCI = dyn_cast<InvokeInst>(&I)) {
//          Function *pF = pCI->getCalledFunction();
//
//					//// Handle calls using function pointers
//					//if (!pF) {
//					//	//errs() << "(1) pII->dump(): "; pCI->dump();
//          //  bool chk = false;
//          //  unsigned idx = 0;
//      		//	for (auto arg = pCI->arg_begin(); arg != pCI->arg_end(); ++arg) {
//					//		if (Value *pV = dyn_cast<Value>(arg)) {
//          //      if (PointerType *pty = dyn_cast<PointerType>(pV->getType())) {
//          //        addXtag(pCI, idx, M);
//          //        chk = true;
//          //      }
//					//		}
//
//          //    idx++;
//          //  }
//					//}
//
//          //if (pF && (pF->isDeclaration() || pF->isVarArg())) {
//          if (pF && pF->isDeclaration()) {
//						bool match = false;
//						for (auto x: white_funcs) {
//							if (pF->getName() == x) {
//								match = true;
//								break;
//							}
//						}
//						if (match)
//							continue;
//
//						//for (auto x: black_funcs) {
//						//	if (pF->getName() == x) {
//						//		match = true;
//						//		break;
//						//	}
//						//}
//						//if (!match)
//						//	continue;
//
//						//errs() << "(2) pII->dump(): "; pCI->dump();
//            bool chk = false;
//            unsigned idx = 0;
//      			for (auto arg = pCI->arg_begin(); arg != pCI->arg_end(); ++arg) {
//							if (Value *pV = dyn_cast<Value>(arg)) {
//                if (PointerType *pty = dyn_cast<PointerType>(pV->getType())) {
//                  addXtag(pCI, idx, M);
//                  chk = true;
//                }
//							}
//
//              idx++;
//            }
//
//						if (chk) {
//							bool chk2 = false;
//							for (auto x: black_funcs) {
//								if (pF->getName() == x) {
//									chk2 = true;
//									break;
//								}
//							}
//							//if (!chk2)
//							//	errs() << "pF->getName(): " << pF->getName() << "\n";
//						} else {
//							//errs() << "pF->getName(): " << pF->getName() << "\n";
//							assert(false);
//						}
//
//            //if (!pF->isDeclaration() && pF->isVarArg())
//            //  errs() << "F.getName(): " << pF->getName() << "\n";
//          }
//        }
//      } 
//    }
//  }
//}



void DataPtrTagPass::insertDptSet(Module &M) {
  //for (auto &F : M) {
  //  if (&F && !F.isDeclaration()) {
  //    auto &BB = F.front();
  //    auto &I = BB.front();
  //    Module *pM = F.getParent();
  //    IRBuilder<> Builder(&I);

  //    Value *arg = ConstantInt::get(Type::getInt64Ty(pM->getContext()), func_num++);
  //    FunctionType *FuncTypeA = FunctionType::get(Type::getVoidTy(pM->getContext()), {Type::getInt64Ty(pM->getContext())}, false);
  //    auto print = F.getParent()->getOrInsertFunction("dpt_print_func", FuncTypeA);
  //    Builder.CreateCall(print, {arg});

	//		for (auto &BB: F) {
	//			for (auto &I: BB) {
	//				if (dyn_cast<ReturnInst>(&I)) {
	//					IRBuilder<> BuilderB(&I);

	//					Value *arg = ConstantInt::get(Type::getInt64Ty(pM->getContext()), func_num-1);
	//					FunctionType *FuncTypeA = FunctionType::get(Type::getVoidTy(pM->getContext()), {Type::getInt64Ty(pM->getContext())}, false);
	//					auto print = F.getParent()->getOrInsertFunction("dpt_print_func_ret", FuncTypeA);
	//					BuilderB.CreateCall(print, {arg});
	//					break;
	//				}
	//			}
	//		}

  //  }
  //}

	// Insert dpt_set() to init configuration
	bool chk = false;
	for (auto &F : M) {
		if (&F && F.getSection().find(".text.startup") != std::string::npos) {
			//errs() << "Found startup function: " << F.getName() << "\n";
			auto &BB = F.front();
			auto &I = BB.front();
			Module *pM = F.getParent();
			IRBuilder<> Builder(&I);

			FunctionType *FuncTypeA = FunctionType::get(Type::getVoidTy(pM->getContext()), false);
			auto init = F.getParent()->getOrInsertFunction("__dpt_set", FuncTypeA);
			Builder.CreateCall(init, {});
			chk = true;
			break;
		}
	}

	// if startup function is found, dpt_set already inserted
	if (chk)
		return;

	for (auto &F : M) {
		if (&F && F.getName() == "main") {
			auto &BB = F.front();
			auto &I = BB.front();
			Module *pM = F.getParent();
			IRBuilder<> Builder(&I);

			FunctionType *FuncTypeA = FunctionType::get(Type::getVoidTy(pM->getContext()), false);
			auto init = F.getParent()->getOrInsertFunction("__dpt_set", FuncTypeA);
			Builder.CreateCall(init, {});
			break;
		}
	}	
}
