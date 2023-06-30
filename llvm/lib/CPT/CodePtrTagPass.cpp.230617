#include "llvm/CPT/CodePtrTagPass.h"
#include "llvm/CPT/CPT.h"

#define PARTS_USE_SHA3

extern "C" {
#include "../PARTS-sha3/include/sha3.h"
}

#define ASSERT(x) if (!(x)) *(voidptr) = 0;

char CodePtrTagPass::ID = 0;
static RegisterPass<CodePtrTagPass> X("cpt-tag", "Code-pointer tagging pass");

Pass *llvm::CPT::createCodePtrTagPass() { return new CodePtrTagPass(); }

bool CodePtrTagPass::runOnModule(Module &M) {
	init(M);
  auto const cptInstType = llvm::CPT::getCptInstType();
  auto const cptQemuMode = llvm::CPT::getCptQemuMode();

  if (cptInstType == CptType::None) {
		insertCptSet(M);
	  return false;
	}

	errs() << "Start CPT pass!\n";

	CPT = (cptInstType == CptType::CPT);
	QEMU = (cptQemuMode == CptQemuEn::Enable);
	ASSERT(CPT);

	FuncTypeCfgPass &MT = getAnalysis<FuncTypeCfgPass>();
	label_map = MT.getLabelMap();
	white_set = MT.getWhiteSet();
	max_label = MT.getMaxLabel();

	replaceUsers(M);
	handleIndirectCalls(M);
	handleGlobalVariables(M);
	initEMT(M);
	if (QEMU)
		handleIntrinsicFunctions(M);

	insertCptSet(M);
	if (QEMU)
	  printFuncAddr(M);

	errs() << "Size of white set: " << white_set.size() << "\n";
	errs() << "statNumFunction: " << statNumFunction << "\n";
	errs() << "statNumIndirectCall: " << statNumIndirectCall << "\n";
	errs() << "statNumESTR: " << statNumESTR << "\n";
	errs() << "statMaxEqClassSize: " << statMaxEqClassSize << "\n";

	return true; // function_modified = true
}

void CodePtrTagPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<FuncTypeCfgPass> ();
}

void CodePtrTagPass::replaceUsers(Module &M) {
	map<Function*,set<Instruction*>*> replace_map;
	map<Function*,set<BitCastOperator*>*> bitcast_map;
	map<Function*,set<PtrToIntOperator*>*> ptrtoint_map;
	map<Function*,set<PHINode*>*> phinode_map;

	for (auto pF: white_set) {
		//errs() << "pF->getName(): " << pF->getName() << "\n";
		if (!replace_map[pF])
			replace_map[pF] = new set<Instruction*>;
		if (!bitcast_map[pF])
			bitcast_map[pF] = new set<BitCastOperator*>;
		if (!ptrtoint_map[pF])
			ptrtoint_map[pF] = new set<PtrToIntOperator*>;
		if (!phinode_map[pF])
			phinode_map[pF] = new set<PHINode*>;

		for (auto pU: pF->users()) {
			if (auto pI = dyn_cast<Instruction>(pU)) {
				if (auto pSI = dyn_cast<StoreInst>(pI)) {
					if (pSI->getValueOperand() == pF) {
						replace_map[pF]->insert(pI);
					}
				} else if (auto pCI = dyn_cast<CallInst>(pI)) {
					if (pCI->getCalledFunction() != pF) {
						replace_map[pF]->insert(pI);
					}
				} else if (auto pII = dyn_cast<InvokeInst>(pI)) {
					if (pII->getCalledFunction() != pF) {
						replace_map[pF]->insert(pI);
					}
				} else if (auto pBC = dyn_cast<BitCastInst>(pI)) {
					ASSERT(false); // 
				} else if (auto pSI = dyn_cast<SelectInst>(pI)) {
					//errs() << "Found SelectInst: "; pI->dump();
					replace_map[pF]->insert(pI);
				} else if (auto pCI = dyn_cast<ICmpInst>(pU)) {
					// Do nothing
				} else if (auto pPN = dyn_cast<PHINode>(pI)) {
					phinode_map[pF]->insert(pPN);
				} else {
					//TODO
					errs() << "else pI->dump(): "; pI->dump();
				}
			} else if (auto pGV = dyn_cast<GlobalVariable>(pU)) {
				//TODO
			} else if (auto pPTI = dyn_cast<PtrToIntOperator>(pU)) {
				errs() << "(a) Found PtrToIntOperator! "; pPTI->dump();
				ptrtoint_map[pF]->insert(pPTI);
			} else if (auto pBC = dyn_cast<BitCastOperator>(pU)) {
				//errs() << "Found BC! "; pBC->dump();
				bitcast_map[pF]->insert(pBC);
				//handleBitCastOperator(M, pBC, pF);
			} else if (auto pConst = dyn_cast<Constant>(pU)) {
			} else {
				errs() << "pU->dump(): "; pU->dump();
			}
		}
	}

	for (auto x: bitcast_map) {
		Function *pF = x.first;
		for (auto pBC: *(x.second)) {
			handleBitCastOperator(M, pBC, pF);
		}
	}

	for (auto x: ptrtoint_map) {
		Function *pF = x.first;
		for (auto pPTI: *(x.second)) {
			handlePtrToIntOperator(M, pPTI, pF);
		}
	}

	for (auto x: phinode_map) {
		Function *pF = x.first;
		for (auto pPN: *(x.second)) {
			handlePHINode(M, pPN, pF, pF);
		}
	}

	for (auto x: replace_map) {
		Function *pF = x.first;
		for (auto pI: *(x.second)) {
			replaceUser(M, pI, pF, pF);
		}
	}
}

void CodePtrTagPass::replaceUser(Module &M, Instruction *pI, Value *pV, Value *pF) {
	u_int16_t label = label_map[pF->getType()];
	if (label != 0) {
		IRBuilder<> Builder(pI);
		Instruction *callA = insertTagc(M, &Builder, pV, pF);
		insertEact(M, &Builder, callA, label);
		auto castA = Builder.CreateCast(Instruction::BitCast, callA, pV->getType());
		replaceOp(pV, castA, pI);
	} else {
		ASSERT(false);
	}
}

Instruction *CodePtrTagPass::insertTagc(Module &M, IRBuilder<> *Builder, Value *pV, Value *pF) {
	FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
  auto typeIdConstant = getTypeIDConstantFrom(*(pF->getType()), *C);

	if (pV->getType()->isPointerTy()) {
		auto castA = Builder->CreateCast(Instruction::BitCast, pV, Type::getInt8PtrTy(*C));
		auto tagc = QEMU? M.getOrInsertFunction("__tagc", FuncTypeA) :
											Intrinsic::getDeclaration(&M, Intrinsic::cpt_tagc, {Type::getInt8PtrTy(*C)});
		auto callA = Builder->CreateCall(tagc, {castA, typeIdConstant}, "");
		return callA;
	} else {
		auto castA = Builder->CreateIntToPtr(pV, Type::getInt8PtrTy(*C));
		auto tagc = QEMU? M.getOrInsertFunction("__tagc", FuncTypeA) :
											Intrinsic::getDeclaration(&M, Intrinsic::cpt_tagc, {Type::getInt8PtrTy(*C)});
		auto callA = Builder->CreateCall(tagc, {castA, typeIdConstant}, "");

		return callA;
	}
}

Instruction *CodePtrTagPass::insertXtag(Module &M, IRBuilder<> *Builder, Value *pV) {
	FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C)}, false);

	if (pV->getType()->isPointerTy()) {
		auto castA = Builder->CreateCast(Instruction::BitCast, pV, Type::getInt8PtrTy(*C));
		auto xtag = M.getOrInsertFunction("__xtag", FuncTypeA);
		//auto xtag = QEMU? M.getOrInsertFunction("__xtag", FuncTypeA) :
		//									Intrinsic::getDeclaration(&M, Intrinsic::cpt_xtag, {Type::getInt8PtrTy(*C)});
		auto callA = Builder->CreateCall(xtag, {castA}, "");
		return callA;
	} else {
		auto castA = Builder->CreateIntToPtr(pV, Type::getInt8PtrTy(*C));
		auto xtag = M.getOrInsertFunction("__xtag", FuncTypeA);
		//auto xtag = QEMU? M.getOrInsertFunction("__xtag", FuncTypeA) :
		//									Intrinsic::getDeclaration(&M, Intrinsic::cpt_xtag, {Type::getInt8PtrTy(*C)});
		auto callA = Builder->CreateCall(xtag, {castA}, "");
		return callA;
	}
}

void CodePtrTagPass::insertEact(Module &M, IRBuilder<> *Builder, Value *pV, size_t label) {
	FunctionType *FuncTypeA = FunctionType::get(Type::getVoidTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
	Value *argB = ConstantInt::get(Type::getInt64Ty(*C), label);
	auto eact = QEMU? M.getOrInsertFunction("__eact", FuncTypeA) :
										Intrinsic::getDeclaration(&M, Intrinsic::cpt_eact);
	Builder->CreateCall(eact, {pV, argB}, "");
}

void CodePtrTagPass::insertEstr(Module &M, IRBuilder<> *Builder, Value *pV, size_t label) {
	FunctionType *FuncTypeA = FunctionType::get(Type::getVoidTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
	Value *argB = ConstantInt::get(Type::getInt64Ty(*C), label);
	auto estr = QEMU? M.getOrInsertFunction("__estr", FuncTypeA) :
										Intrinsic::getDeclaration(&M, Intrinsic::cpt_estr);
	Builder->CreateCall(estr, {pV, argB}, "");
}

void CodePtrTagPass::replaceOp(Value *pVa, Value *pVb, Instruction *pI) {
	//errs() << "pVa->dump(): "; pVa->dump();
	//errs() << "pVb->dump(): "; pVb->dump();
	//errs() << "pI->dump(): "; pI->dump();
	unsigned nth = 0;
	bool chk = false;
	for (auto op = pI->op_begin(); op != pI->op_end(); op++) {
		if (auto *_pVa = dyn_cast<Value>(op)) {
			if (_pVa == pVa) {
				pI->setOperand(nth, pVb);
				//errs() << "(2) pI->dump(): "; pI->dump();
				chk = true;
				//break;
			}
		}

		nth++;
	}

	//errs() << "(3) pI->dump(): "; pI->dump();

	ASSERT(chk);
}

void CodePtrTagPass::handleBitCastOperator(Module &M, BitCastOperator *pBC, Function *pF) {
	map<Value*,set<Instruction*>*> replace_map;
	set<PHINode*> phinode_set;

	if (!replace_map[pBC])
		replace_map[pBC] = new set<Instruction*>;

	for (auto pU: pBC->users()) {
		//errs() << "pU->dump(): "; pU->dump();
		if (auto pI = dyn_cast<Instruction>(pU)) {
			if (auto pSI = dyn_cast<StoreInst>(pI)) {
				if (pSI->getValueOperand() == pBC) {
					replace_map[pBC]->insert(pI);
					//replaceUser(M, pSI, pF);
				}
			} else if (auto pCI = dyn_cast<CallInst>(pI)) {
				if (pCI->getCalledFunction() != dyn_cast<Function>(pBC)) {
					replace_map[pBC]->insert(pI);
					//replaceUser(M, pI, pBC);
				}
			} else if (auto pII = dyn_cast<InvokeInst>(pI)) {
				if (pII->getCalledFunction() != dyn_cast<Function>(pBC)) {
					replace_map[pBC]->insert(pI);
					//replaceUser(M, pI, pBC);
				}
			} else if (auto pSI = dyn_cast<SelectInst>(pI)) {
				//errs() << "Found SelectInst: "; pI->dump();
				replace_map[pBC]->insert(pI);
			} else if (auto pPN = dyn_cast<PHINode>(pI)) {
				phinode_set.insert(pPN);
			} else {
				errs() << "(4) pI->dump(): "; pI->dump();
			}
		} else if (auto pGV = dyn_cast<GlobalVariable>(pU)) {
			//errs() << "Found GV! "; pGV->dump();
			//handleGlobalFuncPtr(M, pGV, pBC);
		} else if (auto pPTI = dyn_cast<PtrToIntOperator>(pU)) {
			errs() << "(b) Found PtrToIntOperator! "; pPTI->dump();
			//TODO
		} else if (auto pConst = dyn_cast<Constant>(pU)) {
			//handleConstant(M, pConst, pBC);
		} else {
			errs() << "(a) pU->dump(): "; pU->dump();
		}
	}

	for (auto x: replace_map) {
		Value *pV = x.first;
		for (auto pI: *(x.second)) {
			replaceUser(M, pI, pV, pF);
		}
	}

	for (auto pPN: phinode_set) {
		handlePHINode(M, pPN, pBC, pF);
	}
}

void CodePtrTagPass::handlePtrToIntOperator(Module &M, PtrToIntOperator *pPTI, Function *pF) {
	set<Instruction*> inst_set;
	set<PHINode*> phinode_set;
	map<Constant*, set<Instruction*>*> const_map;

	for (auto pU: pPTI->users()) {
		if (auto pI = dyn_cast<Instruction>(pU)) {
			if (auto pSI = dyn_cast<StoreInst>(pI)) {
				if (pSI->getValueOperand() == pPTI) {
					inst_set.insert(pI);
				}
			} else if (auto pBC = dyn_cast<BitCastInst>(pI)) {
				ASSERT(false); // 
			} else if (auto pSI = dyn_cast<SelectInst>(pI)) {
				inst_set.insert(pI);
			} else if (auto pCI = dyn_cast<ICmpInst>(pI)) {
				// Do nothing
			} else if (auto pPN = dyn_cast<PHINode>(pI)) {
				errs() << "Found PHINode user of pPTI\n";
				phinode_set.insert(pPN);
			} else {
				//TODO
				errs() << "else pI->dump(): "; pI->dump();
			}
		} else if (auto pConst = dyn_cast<Constant>(pU)) {
			if (!const_map[pConst])
				const_map[pConst] = new set<Instruction*>;

			errs() << "pConst->dump(): "; pConst->dump();
			if (pConst->getType()->isArrayTy() || pConst->getType()->isStructTy()) {
				errs() << "(2) pConst->dump(): "; pConst->dump();
				for (auto pU2: pConst->users()) {
					if (auto pI2 = dyn_cast<Instruction>(pU2)) {
						const_map[pConst]->insert(pI2);
						//errs() << "pI2->dump(): "; pI2->dump();
					}
				}
			} else {
			}
		}
	}

	for (auto pI: inst_set) {
		u_int16_t label = label_map[pF->getType()];
		if (label != 0) {
			IRBuilder<> Builder(pI);
			Instruction *callA = insertTagc(M, &Builder, pF, pF);
			insertEact(M, &Builder, callA, label);
			auto castB = Builder.CreatePtrToInt(callA, pPTI->getType());
			replaceOp(pPTI, castB, pI);
		} else {
			ASSERT(false);
		}
	}

	for (auto pPN: phinode_set) {
		handlePHINode(M, pPN, pPTI, pF);
	}

	for (auto x: const_map) {
		Constant *pConst = x.first;
		for (auto pI: *(x.second)) {
			u_int16_t label = label_map[pF->getType()];
			if (label != 0) {
				// TODO is first arg or second?
				IRBuilder<> Builder(pI);
				Instruction *callA = insertTagc(M, &Builder, pF, pF);
				insertEact(M, &Builder, callA, label);

				auto castB = Builder.CreatePtrToInt(callA, pPTI->getType());
				auto undef = UndefValue::get(pConst->getType());
				auto extract = Builder.CreateExtractValue(pConst, 1);
				auto insert = Builder.CreateInsertValue(undef, castB, 0);
				auto insert2 = Builder.CreateInsertValue(insert, extract, 1);

				replaceOp(pConst, insert2, pI);
			} else {
				ASSERT(false);
			}
		}
	}
}

void CodePtrTagPass::handlePHINode(Module &M, PHINode *pPN, Value *pV, Value *pF) {
	//errs() << "Found PHINode: "; pPN->dump();
	unsigned num = pPN->getNumIncomingValues();
	for (unsigned i=0; i<num; i++) {
		if (pPN->getIncomingValue(i) == pV) {
			auto pBB = pPN->getIncomingBlock(i);
			auto &I = pBB->back();

			u_int16_t label = label_map[pF->getType()];
			if (label != 0) {
				IRBuilder<> Builder(&I);
				Instruction *callA = insertTagc(M, &Builder, pV, pF);
				insertEact(M, &Builder, callA, label);

				if (pV->getType()->isPointerTy()) {
					auto castA = Builder.CreateCast(Instruction::BitCast, callA, pV->getType());
					pPN->setIncomingValue(i, castA);
				} else {
					auto castA = Builder.CreatePtrToInt(callA, pV->getType());
					pPN->setIncomingValue(i, castA);
				}
			} else {
				ASSERT(false);
			}
		}
	}
}

void CodePtrTagPass::handleGlobalVariables(Module &M) {
	set<GlobalVariable*> gv_set;

	for (auto &G : M.getGlobalList()) {
		GlobalVariable *pGV = dyn_cast<GlobalVariable>(&G);

		//errs() << "(a) pGV->dump(): "; pGV->dump();
		if (pGV->getName().find(".str") != 0 && pGV->hasInitializer()) {
			gv_set.insert(pGV);
		}
  }

	for (auto pGV: gv_set) {
		//errs() << "pGV->dump(): "; pGV->dump();
		handleGlobalVariable(M, pGV);
	}
}

void CodePtrTagPass::handleGlobalVariable(Module &M, GlobalVariable *pGV) {
	Constant *pConst = pGV->getInitializer();
	Type *ty = pConst->getType();

	vector<Value*> *indices = new vector<Value*>;

	if (Function *pF = dyn_cast<Function>(pConst)) {
		if (white_set.find(pF) != white_set.end())
			handleFunctionTy(M, pF, pGV, indices, pF);
	} else if (auto *pBC = dyn_cast<BitCastOperator>(pConst)) {
		//errs() << "(1) BitCastOp pConst->dump(): "; pConst->dump();
		if (Function *pF = dyn_cast<Function>(pBC->getOperand(0))) {
			if (white_set.find(pF) != white_set.end())
				handleFunctionTy(M, pBC, pGV, indices, pF);
			//errs() << "(1) BitCastOp with Function Pointer pConst->dump(): "; pConst->dump();
		}
	} else if (ty->isArrayTy()) {
		indices->push_back(ConstantInt::get(Type::getInt64Ty(*C), 0));
		handleArrayTy(M, pConst, pGV, indices);
	} else if (ty->isStructTy()) {
		indices->push_back(ConstantInt::get(Type::getInt32Ty(*C), 0));
		handleStructTy(M, pConst, pGV, indices);
	} else if (ty->isPointerTy()) {
		//errs() << "(1) Pointer ty->dump(): "; ty->dump();
	} else {
		//errs() << "(1) else ty->dump(): "; ty->dump();
	}

	pGV->setConstant(false);

	delete indices;
}

void CodePtrTagPass::handleFunctionTy(Module &M, Value *pV, GlobalVariable *pGV, vector<Value*> *indices, Value *pF) {
	//errs() << "Handle Function: " << pF->getName() << "\n";
	auto &BB = entry->front();
	auto &I = BB.back();
	IRBuilder<> Builder(&I);

	//errs() << "pGV->dump(): "; pGV->dump();
	//for (auto x: *indices) {
	//	x->dump();
	//}

	u_int16_t label = label_map[pF->getType()];
	if (label != 0) {
		Instruction *callA = insertTagc(M, &Builder, pV, pF);
		if (visit_set.find(pF) == visit_set.end()) {
			insertEact(M, &Builder, callA, label);
			visit_set.insert(pF);
		}
		auto castB = Builder.CreateCast(Instruction::BitCast, callA, pV->getType());

		auto gep = Builder.CreateGEP(pGV, *indices);
		auto store = Builder.CreateStore(castB, gep);
		store->setAlignment(8);
	} else {
		ASSERT(false);
	}
	//errs() << "gep->dump(): "; gep->dump();
	//errs() << "store->dump(): "; store->dump();
}

void CodePtrTagPass::handleArrayTy(Module &M, Constant *pConst, GlobalVariable *pGV, vector<Value*> *indices) {
	Type *ty = pConst->getType();
	unsigned n = ty->getArrayNumElements();
	Type *_ty = ty->getArrayElementType();
	//errs() << "pConst->dump(): "; pConst->dump();
	//errs() << "Handle Array Ty(" << n << "): "; ty->dump();

	for (unsigned i=0; i<n; i++) {
		Constant *_pConst = pConst->getAggregateElement(i);
		vector<Value*> *indices_new = new vector<Value*>;
		//errs() << "Array[" << i << "] Aggregate Element\n";
		//errs() << "_pConst->dump(): "; _pConst->dump();

		// Copy indices
		for (auto pV: *indices)
			indices_new->push_back(pV);

		indices_new->push_back(ConstantInt::get(Type::getInt64Ty(*C), i));

		// Check element type
		if (auto *_pF = dyn_cast<Function>(_pConst)) {
			if (white_set.find(_pF) != white_set.end())
				handleFunctionTy(M, _pF, pGV, indices_new, _pF);
		} else if (auto *_pGA = dyn_cast<GlobalAlias>(_pConst)) {
			if (auto *_pF = dyn_cast<Function>(_pGA->getAliasee())) {
				if (white_set.find(_pF) != white_set.end())
					handleFunctionTy(M, _pGA, pGV, indices_new, _pF);
				//errs() << "(1) Found!!!\n";
			}
		} else if (auto *_pBC = dyn_cast<BitCastOperator>(_pConst)) {
			if (auto *_pF = dyn_cast<Function>(_pBC->getOperand(0))) {
				if (white_set.find(_pF) != white_set.end())
					handleFunctionTy(M, _pBC, pGV, indices_new, _pF);
			} else if (auto *_pGA = dyn_cast<GlobalAlias>(_pBC->getOperand(0))) {
				if (auto *_pF = dyn_cast<Function>(_pGA->getAliasee())) {
					if (white_set.find(_pF) != white_set.end())
						handleFunctionTy(M, _pBC, pGV, indices_new, _pF);
					//errs() << "(2) Found!!!\n";
				}
			}
		} else if (_ty->isArrayTy()) {
			handleArrayTy(M, _pConst, pGV, indices_new);
		} else if (_ty->isStructTy()) {
			handleStructTy(M, _pConst, pGV, indices_new);
		} else {
			//errs() << "(1) else _ty->dump(): "; _ty->dump();
		}

		delete indices_new;
	}
}

void CodePtrTagPass::handleStructTy(Module &M, Constant *pConst, GlobalVariable *pGV, vector<Value*> *indices) {
	Type *ty = pConst->getType();
	unsigned n = dyn_cast<StructType>(ty)->getNumElements();
	//errs() << "pConst->dump(): "; pConst->dump();
	//errs() << "Handle Struct Ty(n:" << n << "): "; ty->dump();

	for (unsigned i=0; i<n; i++) {
		Constant *_pConst = pConst->getAggregateElement(i);
		Type *_ty = dyn_cast<StructType>(ty)->getElementType(i);
		vector<Value*> *indices_new = new vector<Value*>;
		//errs() << "Struct[" << i << "] Aggregate Element\n";
		//errs() << "_pConst->dump(): "; _pConst->dump();
		//errs() << "_ty->dump(): "; _ty->dump();

		// Copy indices
		for (auto pV: *indices)
			indices_new->push_back(pV);

		indices_new->push_back(ConstantInt::get(Type::getInt32Ty(*C), i));

		// Check element type
		if (Function *_pF = dyn_cast<Function>(_pConst)) {
			if (white_set.find(_pF) != white_set.end())
				handleFunctionTy(M, _pF, pGV, indices_new, _pF);
		} else if (auto *_pGA = dyn_cast<GlobalAlias>(_pConst)) {
			if (auto *_pF = dyn_cast<Function>(_pGA->getAliasee())) {
				if (white_set.find(_pF) != white_set.end())
					handleFunctionTy(M, _pGA, pGV, indices_new, _pF);
				errs() << "(3) Found!!!\n";
			}
		} else if (auto *_pBC = dyn_cast<BitCastOperator>(_pConst)) {
			if (Function *_pF = dyn_cast<Function>(_pBC->getOperand(0))) {
				if (white_set.find(_pF) != white_set.end())
					handleFunctionTy(M, _pBC, pGV, indices_new, _pF);
			} else if (auto *_pGA = dyn_cast<GlobalAlias>(_pBC->getOperand(0))) {
				if (auto *_pF = dyn_cast<Function>(_pGA->getAliasee())) {
					if (white_set.find(_pF) != white_set.end())
						handleFunctionTy(M, _pBC, pGV, indices_new, _pF);
					errs() << "(4) Found!!!\n";
				}
			}
		} else if (_ty->isArrayTy()) {
			handleArrayTy(M, _pConst, pGV, indices_new);
		} else if (_ty->isStructTy()) {
			handleStructTy(M, _pConst, pGV, indices_new);
		//} else if (_ty->isPointerTy()) {
		} else {
			//errs() << "(2) else _ty->dump(): "; _ty->dump();
		}

		delete indices_new;
	}
}

void CodePtrTagPass::initEMT(Module &M) {
	auto &BB = entry->front();
	auto &I = BB.front();
	IRBuilder<> Builder(&I);

	FunctionType *FuncTypeA = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);
	FunctionType *FuncTypeB = FunctionType::get(Type::getVoidTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C)}, false);

	for (auto pF: white_set) {
		u_int16_t label = label_map[pF->getType()];
		ASSERT(label != 0);

		// Insert tagd rd, rs1, rs2
		// Insert estr rs1, rs2
		Instruction *callA = insertTagc(M, &Builder, pF, pF);
		insertEstr(M, &Builder, callA, label);
		statNumESTR++;
	}

	// Stats
  unsigned *cnt_arr = new unsigned[max_label];

  for (u_int16_t i=0; i<max_label; i++)
    cnt_arr[i] = 0;

  for (auto pF: white_set) {
    u_int16_t _label = label_map[pF->getType()];
    if (_label == 0) {
      pF->dump();
    }
    ASSERT(_label != 0);
    cnt_arr[_label]++;
  }

  statMaxEqClassSize = 0;
  for (u_int16_t i=0; i<max_label; i++) {
    if (statMaxEqClassSize < cnt_arr[i])
      statMaxEqClassSize = cnt_arr[i];
  }

	delete[] cnt_arr;

	for (auto &F: M) {
		statNumFunction++;
	}
}

void CodePtrTagPass::handleIndirectCalls(Module &M) {
	set<Instruction*> call_set;

	for (auto &F: M) {
    // Skip startup code
		if (F.getSection().find(".text.startup") != std::string::npos)
      continue;

		for (auto &BB : F) {
			Function *caller = &F;

			for (auto &I : BB) {
        if (!isIndirectCall(&I))
          continue;

				call_set.insert(&I);
				statNumIndirectCall++;
			}
		}
	}

	for (auto pI: call_set) {
		// Insert echk before pCI
		handleIndirectCall(M, pI);
	}
}

void CodePtrTagPass::handleIndirectCall(Module &M, Instruction *pI) {
	Value* callee = NULL;
	if (CallInst *pCI = dyn_cast<CallInst>(pI))
		callee = pCI->getCalledValue();
	else if (InvokeInst *pII = dyn_cast<InvokeInst>(pI))
		callee = pII->getCalledValue();

	//errs() << "pI->dump(): "; pI->dump();
	FunctionType *fty1 = NULL;
	if (CallInst *pCI = dyn_cast<CallInst>(pI))
		fty1 = pCI->getFunctionType();
	else if (InvokeInst *pII = dyn_cast<InvokeInst>(pI)) {
		fty1 = pII->getFunctionType();
		//errs() << "pII->dump(): "; pII->dump();
		//errs() << "fty1->dump(): "; fty1->dump();
	} else
		ASSERT(false);

	//FunctionType *fty1 = pCI->getFunctionType();
	u_int16_t label = 0;

	if (fty1->isVarArg()) {
		vector<Type*> indices;
		//errs() << "pI->dump(): "; pI->dump();
		for (unsigned i = 0; i < pI->getNumOperands()-1; i++) {
			pI->getOperand(i)->dump();
			indices.push_back(pI->getOperand(i)->getType());
		}
		fty1 = FunctionType::get(fty1->getReturnType(), indices, false);
	}

	PointerType *pty1 = PointerType::get(fty1, 0);
	label = label_map[pty1];
	if (label == 0) {
		if (QEMU) {
			IRBuilder<> Builder(pI);
			FunctionType *FuncTypeB = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C)}, false);
			auto castA = Builder.CreateCast(Instruction::BitCast, callee, Type::getInt8PtrTy(*C));
			auto xtag = M.getOrInsertFunction("__xtag", FuncTypeB);
			auto callA = Builder.CreateCall(xtag, {castA}, "");
			auto castB = Builder.CreateCast(Instruction::BitCast, callA, callee->getType());
			replaceOp(callee, castB, pI);
		}

		return;
	}

	IRBuilder<> Builder(pI);

	// Insert echk rs1, rs2
	FunctionType *FuncTypeA = FunctionType::get(Type::getVoidTy(*C), {Type::getInt8PtrTy(*C), Type::getInt64Ty(*C), Type::getInt64Ty(*C)}, false);
	FunctionType *FuncTypeB = FunctionType::get(Type::getInt8PtrTy(*C), {Type::getInt8PtrTy(*C)}, false);
  Value *arg0 = ConstantInt::get(Type::getInt64Ty(*C), label);
  Value *arg1 = ConstantInt::get(Type::getInt64Ty(*C), temp_cnt++);
	auto castA = Builder.CreateCast(Instruction::BitCast, callee, Type::getInt8PtrTy(*C));
	auto echk = QEMU? M.getOrInsertFunction("__echk", FuncTypeA) :
										Intrinsic::getDeclaration(&M, Intrinsic::cpt_echk);

	if (QEMU) {
		Builder.CreateCall(echk, {castA, arg0, arg1}, "");
		auto xtag = M.getOrInsertFunction("__xtag", FuncTypeB);
		auto callA = Builder.CreateCall(xtag, {castA}, "");
		auto castB = Builder.CreateCast(Instruction::BitCast, callA, callee->getType());
		replaceOp(callee, castB, pI);
		//bool check = false;
		//unsigned nth = 0;
		//for (auto op = pI->op_begin(); op != pI->op_end(); op++, nth++) {
		//	if (dyn_cast<Value>(op) == callee) {
		//		pI->setOperand(nth, castB);
		//		check = true;
		//		break;
		//	}
		//}

		//ASSERT(check);
	} else {
		Builder.CreateCall(echk, {castA, arg0}, "");
	}
}

bool CodePtrTagPass::isIndirectCall(Instruction *pI) {
  Function *pF = nullptr;

	if (CallInst *pCI = dyn_cast<CallInst>(pI)) {
    pF = pCI->getCalledFunction();

    if (!pF) {
			FunctionType *fty = pCI->getFunctionType();
			//errs() << "pCI->dump(): "; pCI->dump();
			//errs() << "pCI->numOperands(): "; pCI->getNumOperands();
			//errs() << "fty->dump(): "; fty->dump();
			//errs() << "isVarArg(): " << fty->isVarArg() << "\n";
			//errs() << "getNumParams(): " << fty->getNumParams() << "\n";
			//errs() << "params()->size(): " << fty->params().size() << "\n";

      auto op = pCI->op_end();
      auto func = (--op)->get();

      // Global alias of function is not an indirect call
      if (auto pGA = dyn_cast<GlobalAlias>(func)) {
        return false;
      } 
      
      if (isa<InlineAsm>(pCI->getCalledValue()))
        return false;

      // Skip callinst (bitcast(@func, *))
      auto pV = pCI->getCalledValue();
      if (dyn_cast<Instruction>(pV)) {
      } else if (auto bc_op = dyn_cast<BitCastOperator>(pV)) {
				//errs() << "pCI->dump(): "; pCI->dump();
        if (Function *func = dyn_cast<Function>(bc_op->getOperand(0))) {
          //errs() << "func->getName(): " << func->getName() << "\n";
          return false;
        }

				if (GlobalAlias *alias = dyn_cast<GlobalAlias>(bc_op->getOperand(0))) {
          //errs() << "alias->getName(): " << alias->getName() << "\n";
					return false;
				} 
      }

      return true;
    }

    return false;
  } else if (InvokeInst *pII = dyn_cast<InvokeInst>(pI)) {
    pF = pII->getCalledFunction();

    if (!pF) {
			//errs() << "pII->dump(): "; pII->dump();
      auto op = pII->op_end();
      auto func = (--op)->get();

      // Global alias of function is not an indirect call
      if (auto pGA = dyn_cast<GlobalAlias>(func)) {
        return false;
      } 
      
      if (isa<InlineAsm>(pII->getCalledValue()))
        return false;

      // Skip callinst (bitcast(@func, *))
      auto pV = pII->getCalledValue();
      if (dyn_cast<Instruction>(pV)) {
      } else if (auto bc_op = dyn_cast<BitCastOperator>(pV)) {
        if (Function *func = dyn_cast<Function>(bc_op->getOperand(0))) {
          //errs() << "func->dump(): "; func->dump();
          return false;
        }

				if (GlobalAlias *alias = dyn_cast<GlobalAlias>(bc_op->getOperand(0))) {
          //errs() << "alias->getName(): " << alias->getName() << "\n";
					return false;
				} 
      }

      return true;
    }
  }

  return false;
}

void CodePtrTagPass::handleIntrinsicFunctions(Module &M) {
	for (auto &F: M) {
    for (auto &BB: F) {
      for (auto &I: BB) {
				Function *pF = nullptr;

        if (CallInst *pCI = dyn_cast<CallInst>(&I))
          pF = pCI->getCalledFunction();
        else if (InvokeInst *pII = dyn_cast<InvokeInst>(&I))
          pF = pII->getCalledFunction();

				if (pF && pF->isDeclaration()) {
					if (CallInst *pCI = dyn_cast<CallInst>(&I)) {
						unsigned n = pCI->getNumOperands();

						for (unsigned i=0; i<n-1; i++) {
							Value *pV = pCI->getOperand(i);
							Type *ty = pV->getType();

							if (auto pty = dyn_cast<PointerType>(ty)) {
								if (pty->getElementType()->isFunctionTy()) {
									IRBuilder<> Builder(&I);		
									Instruction *callA = insertXtag(M, &Builder, pV);
									auto castA = Builder.CreateCast(Instruction::BitCast, callA, pV->getType());
									pCI->setOperand(i, castA);
								}
							}
						}
					} else if (InvokeInst *pII = dyn_cast<InvokeInst>(&I)) {
						unsigned n = pII->getNumOperands();

						for (unsigned i=0; i<n-1; i++) {
							Value *pV = pII->getOperand(i);
							Type *ty = pV->getType();

							if (auto pty = dyn_cast<PointerType>(ty)) {
								if (pty->getElementType()->isFunctionTy()) {
									IRBuilder<> Builder(&I);		
									Instruction *callA = insertXtag(M, &Builder, pV);
									auto castA = Builder.CreateCast(Instruction::BitCast, callA, pV->getType());
									pII->setOperand(i, castA);
								}
							}
						}
					}
        }
      }
    }
  }
}

void CodePtrTagPass::buildTypeString(const Type *T, llvm::raw_string_ostream &O) {
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


uint64_t CodePtrTagPass::getTypeIDFor(const Type *T) {
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

Constant *CodePtrTagPass::getTypeIDConstantFrom(const Type &T, LLVMContext &C) {
  return Constant::getIntegerValue(Type::getInt64Ty(C),
                                   //APInt(64, getTypeIDFor(&T) & 0xFFFF));
                                   APInt(64, getTypeIDFor(&T)));

  //return Constant::getIntegerValue(Type::getInt64Ty(C),
  //                                 APInt(64, 0));
}

void CodePtrTagPass::init(Module &M) {
	Function *start = nullptr;

	for (auto &F : M) {
		if (&F && F.getSection().find(".text.startup") != std::string::npos) {
			start = &F;
			break;
		}
	}

	for (auto &F : M) {
		if (&F && F.getName() == "main") {
		  C = &F.getContext();
			DL = &F.getParent()->getDataLayout();
			main = &F;
			if (!start)
				start = main;
			break;
		}
	}

	FunctionType *FuncTypeA = FunctionType::get(Type::getVoidTy(*C), false);
	entry = Function::Create(FuncTypeA, Function::ExternalLinkage, "__init_cpt", M);
	auto pBB = BasicBlock::Create(*C, "", entry, nullptr);
	ReturnInst::Create(*C, pBB);

	auto &BB = start->front();
	auto &I = BB.front();
	IRBuilder<> Builder(&I);		
	Builder.CreateCall(FuncTypeA, entry);
}

void CodePtrTagPass::printFuncAddr(Module &M) {
	auto &BB = entry->front();
	auto &I = BB.front();
	IRBuilder<> Builder(&I);

	for (auto &F : M) {
		if (&F && !F.isDeclaration()) {
			Constant *name = ConstantDataArray::getString(*C, F.getName(), true);

			GlobalVariable* pGV = new GlobalVariable(M, 
        /*Type=*/ name->getType(),
        /*isConstant=*/ true,
        /*Linkage=*/ GlobalValue::PrivateLinkage,
        /*Initializer=*/ 0, // has initializer, specified below
        /*Name=*/ ".func_name");
			pGV->setAlignment(1);
			pGV->setInitializer(name);

			auto castA = Builder.CreateCast(Instruction::BitCast, pGV, Type::getInt8PtrTy(*C));
			auto castB = Builder.CreateCast(Instruction::BitCast, &F, Type::getInt8PtrTy(*C));
			FunctionType *FuncTypeA = FunctionType::get(Type::getVoidTy(M.getContext()), {Type::getInt8PtrTy(*C), Type::getInt8PtrTy(*C)}, false);
			auto print = M.getOrInsertFunction("__print_func", FuncTypeA);
			Builder.CreateCall(print, {castA, castB});
		}
	}
}

void CodePtrTagPass::insertCptSet(Module &M) {
  //for (auto &F : M) {
  //  if (&F && !F.isDeclaration()) {
  //    auto &BB = F.front();
  //    auto &I = BB.front();
  //    IRBuilder<> Builder(&I);

  //    Value *arg = ConstantInt::get(Type::getInt64Ty(M.getContext()), func_num++);
  //    FunctionType *FuncTypeA = FunctionType::get(Type::getVoidTy(M.getContext()), {Type::getInt64Ty(M.getContext())}, false);
  //    auto print = F.getParent()->getOrInsertFunction("dpt_print_func", FuncTypeA);
  //    Builder.CreateCall(print, {arg});

	//		for (auto &BB: F) {
	//			for (auto &I: BB) {
	//				if (dyn_cast<ReturnInst>(&I)) {
	//					IRBuilder<> BuilderB(&I);

	//					Value *arg = ConstantInt::get(Type::getInt64Ty(M.getContext()), func_num-1);
	//					FunctionType *FuncTypeA = FunctionType::get(Type::getVoidTy(M.getContext()), {Type::getInt64Ty(M.getContext())}, false);
	//					auto print = F.getParent()->getOrInsertFunction("dpt_print_func_ret", FuncTypeA);
	//					BuilderB.CreateCall(print, {arg});
	//					break;
	//				}
	//			}
	//		}

  //  }
  //}

	// Insert cpt_set() to init configuration
	auto &BB = entry->front();
	auto &I = BB.front();
	IRBuilder<> Builder(&I);

	FunctionType *FuncTypeA = FunctionType::get(Type::getVoidTy(M.getContext()), false);
	auto init = M.getOrInsertFunction("__cpt_set", FuncTypeA);
	Builder.CreateCall(init);
}
