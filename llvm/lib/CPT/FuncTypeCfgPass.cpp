#include "llvm/CPT/FuncTypeCfgPass.h"
#include "llvm/CPT/CPT.h"

#define ASSERT(x) if (!(x)) *(voidptr) = 0;

char FuncTypeCfgPass::ID = 0;
static RegisterPass<FuncTypeCfgPass> X("cpt-cfg", "Type-based cfg pass");

Pass *llvm::CPT::createFuncTypeCfgPass() { return new FuncTypeCfgPass(); }

bool FuncTypeCfgPass::runOnModule(Module &M) {
	init(M);
  auto const cptInstType = llvm::CPT::getCptInstType();
  auto const cptQemuMode = llvm::CPT::getCptQemuMode();

  if (cptInstType == CptType::None) {
	  return false;
	}

	errs() << "Start Func Type CFG pass!\n";

  initBlackSet();
	findWhiteSet(M);

	return false;
  findTypeAlias(M);
  handleIndirectCalls(M);
	initLabelMap(M);

	//errs() << "statMaxEqClassSize: " << statMaxEqClassSize << "\n";
	//errs() << "statNumIndirectCall: " << statNumIndirectCall << "\n";
	//errs() << "statNumESTR: " << statNumESTR << "\n";

	return false; // function_modified = false
}

map<Type*,u_int16_t> FuncTypeCfgPass::getLabelMap() {
  return label_map;
}

set<Function*> FuncTypeCfgPass::getWhiteSet() {
  return white_set;
}

u_int16_t FuncTypeCfgPass::getMaxLabel() {
  return max_label;
}

void FuncTypeCfgPass::initBlackSet() {
	black_set.insert("llvm.va_start");
	black_set.insert("llvm.va_end");
	black_set.insert("llvm.lifetime.start.p0i8");
	black_set.insert("llvm.lifetime.end.p0i8");
	black_set.insert("llvm.dbg.declare");
	black_set.insert("llvm.dbg.label");
	black_set.insert("llvm.prefetch");
	black_set.insert("llvm.stacksave");
	black_set.insert("llvm.stackrestore");
	black_set.insert("llvm.floor.f64");
	black_set.insert("llvm.floor.f32");
	black_set.insert("llvm.ceil.f64");
	black_set.insert("llvm.ceil.f32");
	black_set.insert("llvm.fabs.f64");
	black_set.insert("llvm.fabs.f32");
	black_set.insert("llvm.va_copy");
	black_set.insert("llvm.round.f64");
	black_set.insert("llvm.trap");
}

void FuncTypeCfgPass::findWhiteSet(Module &M) {
	for (auto &F: M) {
    // Skip blacklisted functions
		bool match = false;
		for (auto x: black_set) {
			if (F.getName() == x) {
				match = true;
				break;
			}
		}

		if (match)
			continue;

    // No need to include startup code
		if (F.getSection().find(".text.startup") != std::string::npos)
      continue;

		for (auto pU: F.users()) {
			if (auto pSI = dyn_cast<StoreInst>(pU)) {
				if (pSI->getValueOperand() == &F)
					white_set.insert(&F);
			} else if (auto pCI = dyn_cast<CallInst>(pU)) {
				if (pCI->getCalledFunction() != &F)
					white_set.insert(&F);
			} else if (auto pII = dyn_cast<InvokeInst>(pU)) {
				if (pII->getCalledFunction() != &F)
					white_set.insert(&F);
			} else if (dyn_cast<SelectInst>(pU)) {
				white_set.insert(&F);
			} else if (dyn_cast<PHINode>(pU)) {
				white_set.insert(&F);
			} else if (dyn_cast<GlobalVariable>(pU)) {
				white_set.insert(&F);
			} else if (dyn_cast<BitCastOperator>(pU)) {
				white_set.insert(&F);
			} else if (dyn_cast<Constant>(pU)) {
				white_set.insert(&F);
			}
		}
	}

	map<unsigned, list<Function*>*> function_table;

	for (auto pF: white_set) {
		unsigned num_param = pF->getFunctionType()->getNumParams();

		if (!function_table[num_param])
			function_table[num_param] = new list<Function*>;

		function_table[num_param]->push_back(pF);
	}

	for (auto x: function_table) {
		unsigned num_param = x.first;
		list<Function*> *flist = x.second;

		if (!flist)
			continue;

		uint64_t num_func = flist->size();

		for (auto pF: *flist) {
			num_func++;
		

			GlobalVariable *pGV = new GlobalVariable(M, 
																	ArrayType::get(Type::getInt8PtrTy(*C), num_func), 				// Type
																	false, 											// isConstant
																	GlobalValue::CommonLinkage, // Linkage
																	0, 												  // Initializer
																	"arity"); 									// Name

			pGV->setAlignment(8);
			ConstantPointerNull
	
			vector<Value *> indices;
			indices.push_back(ConstantInt::get(Type::getInt64Ty(*C), zero));
			Value *zero = ConstantInt::get(Type::getInt64Ty(*C), offset);

			Builder.CreateGEP();
			[pF] = 

			offset++;
		}
	}
	
GlobalVariable* gvar_ptr_abc = new GlobalVariable(/*Module=*/*mod, 
        /*Type=*/PointerTy_0,
        /*isConstant=*/false,
        /*Linkage=*/GlobalValue::CommonLinkage,
        /*Initializer=*/0, // has initializer, specified below
        /*Name=*/"abc");
gvar_ptr_abc->setAlignment(4);

// Constant Definitions
ConstantPointerNull* const_ptr_2 = ConstantPointerNull::get(PointerTy_0);

// Global Variable Definitions
gvar_ptr_abc->setInitializer(const_ptr_2);



	//Builder.CreateCall(init);

	//errs() << "Size of white set: " << white_set.size() << "\n";
	//for (auto pF: white_set) {
	//	errs() << "--" << pF->getName() << "\n";
	//}
}

void FuncTypeCfgPass::findTypeAlias(Module &M) {
	// 1. Find struct type alias...
	list<StructType*> sty_list;

  // Get struct type list
	for (auto &sty: M.getIdentifiedStructTypes()) {
		sty_list.push_back(sty);
	}

	for (auto it = sty_list.begin(); it != sty_list.end(); it++) {
		StructType *sty0 = (*it);

		for (auto it2 = it; it2 != sty_list.end(); it2++) {
			StructType *sty1 = (*it2);

			if (sty0 == sty1)
				continue;

			bool check = false;
			if (sty0->isOpaque() || sty1->isOpaque() ||
					sty0->isLayoutIdentical(sty1)) {
				check = true;
			} else {
				auto name0 = sty0->getName();
				auto name1 = sty1->getName();
				size_t len0 = name0.size();
				size_t len1 = name1.size();
				const char *str0 = name0.data();
				const char *str1 = name1.data();

				size_t len = len0 < len1 ? len0 : len1;
				size_t i = 0;
				for (i=0; i<len; i++) {
					if (str0[i] != str1[i])
						break;
				}

				if (i == len)
					check = true;
			}

			if (check) {
				PointerType *pty0 = PointerType::get(sty0, 0);
				PointerType *pty1 = PointerType::get(sty1, 0);

        add_typeset(pty0, pty1);
			}
		}
	}

	// 2. Find alias from bitcast
	for (auto &F: M) {
		for (auto pU: F.users()) {
			if (auto pBC = dyn_cast<BitCastOperator>(pU)) {
				Type *src_ty = pBC->getSrcTy();
				Type *dst_ty = pBC->getDestTy();

				addTypeAlias(src_ty, dst_ty);
			}
		}

		for (auto &BB: F) {
			for (auto &I: BB) {
				if (auto pBC = dyn_cast<BitCastInst>(&I)) {
					Type *src_ty = pBC->getSrcTy();
					Type *dst_ty = pBC->getDestTy();

					addTypeAlias(src_ty, dst_ty);
				}
			}
		}
	}

	// 3. Add arg alias
  for (auto x: typeset_map) {
		PointerType *pty0 = dyn_cast<PointerType>(x.first);

		if (pty0 == NULL)
			continue;

		Type *ety0 = pty0->getElementType();

		if (!ety0->isFunctionTy())
			continue;

		for (auto y: *(x.second)) {
			PointerType *pty1 = dyn_cast<PointerType>(y);

			if (pty1 == NULL)
				continue;

			Type *ety1 = pty1->getElementType();

			if (!ety1->isFunctionTy())
				continue;

			addArgAlias(ety0, ety1);
		}
  }

  // 4. Add from white set
	for (auto x: white_set) {
		auto ty = x->getType();
    if (!typeset_map[ty]) {
      auto new_typeset = new set<Type*>;
      new_typeset->insert(ty);
      typeset_list.push_back(new_typeset);
      typeset_map[ty] = new_typeset;
    }
	}

  // 5. Try to merge more using type alias info
  for (auto it0 = typeset_map.begin(); it0 != typeset_map.end(); it0++) {
    PointerType *pty0 = dyn_cast<PointerType>(it0->first);
    Type *ety0 = pty0->getElementType();
    if (!pty0 || !ety0->isFunctionTy())
      continue;

		FunctionType *fty0 = dyn_cast<FunctionType>(ety0);
    
    for (auto it1 = it0; it1 != typeset_map.end(); it1++) {
      if (it0 == it1)
        continue;

      PointerType *pty1 = dyn_cast<PointerType>(it1->first);
      Type *ety1 = pty1->getElementType();
      if (!pty1 || !ety1->isFunctionTy())
        continue;

      // Skip if in the same set
      if (typeset_map[pty0] == typeset_map[pty1])
        continue;

  		FunctionType *fty1 = dyn_cast<FunctionType>(ety1);

			if (doTypeComparison(fty0, fty1)) {
        merge_typeset(pty0, pty1);
        break;
      }
    }
  }
}

void FuncTypeCfgPass::addTypeAlias(Type *src_ty, Type *dst_ty) {
	if (!src_ty->isPointerTy() || !dst_ty->isPointerTy()) {
		return;
	}

	PointerType *src_pty = dyn_cast<PointerType>(src_ty);
	PointerType *dst_pty = dyn_cast<PointerType>(dst_ty);

	if (!src_pty->getElementType()->isFunctionTy() &&
			!src_pty->getElementType()->isStructTy()) {
		return;
	}

	if (!dst_pty->getElementType()->isFunctionTy() &&
			!dst_pty->getElementType()->isStructTy())
		return;

  add_typeset(src_pty, dst_pty);
}

void FuncTypeCfgPass::addArgAlias(Type *src_ety, Type *dst_ety) {
	FunctionType *src_fty = dyn_cast<FunctionType>(src_ety);
	FunctionType *dst_fty = dyn_cast<FunctionType>(dst_ety);

	if (src_fty->getNumParams() != dst_fty->getNumParams())
		return;

	unsigned params_num = src_fty->getNumParams();
	for (unsigned i=0; i<params_num; i++) {
		bool check = false;
		Type *ty0 = src_fty->getParamType(i);
		Type *ty1 = dst_fty->getParamType(i);

		PointerType *pty0 = dyn_cast<PointerType>(ty0);
		PointerType *pty1 = dyn_cast<PointerType>(ty1);

		if (pty0 == NULL || pty1 == NULL)
			continue;

		Type *ety0 = pty0->getElementType();
		Type *ety1 = pty1->getElementType();

		if (!ety0->isFunctionTy() && !ety0->isStructTy())
			continue;

		if (!ety1->isFunctionTy() && !ety1->isStructTy())
			continue;

		if (ety0 == ety1)
			continue;

    add_typeset(pty0, pty1);
	}
}

void FuncTypeCfgPass::initLabelMap(Module &M) {
  u_int16_t label = 1;
  for (auto typeset: typeset_list) {
    if (statMaxEqClassSize < typeset->size())
      statMaxEqClassSize = typeset->size();

    bool chk = false;
    for (auto ty: *typeset) {
      PointerType *pty = dyn_cast<PointerType>(ty);
      if (pty->getElementType()->isFunctionTy()) {
        chk = true;
        label_map[ty] = label;
        //errs() << "[Label:" << label << "]: "; ty->dump();
      }
    }

    if (chk)
      label++;
  }

  max_label = label;
}

bool FuncTypeCfgPass::doTypeComparison(FunctionType *fty0, FunctionType *fty1) {
	ASSERT(fty0 != NULL && fty1 != NULL);

	// 0. Check exact match
	if (fty0 == fty1)
		return true;

	// 1. Check func type alias
	PointerType* pty0 = PointerType::get(fty0, 0);
	PointerType* pty1 = PointerType::get(fty1, 0);

  if (typeset_map[pty0]) {
    for (auto _ty: *(typeset_map[pty0])) {
      if (_ty == pty1)
        return true;
    }
  }

	// 2. Check # params
	if (fty0->getNumParams() != fty1->getNumParams())
		return false;

	// 3. Check return type
	//Type *func_ret = fty0->getReturnType();
	//Type *callee_ret = fty1->getReturnType();

	//if (func_ret != callee_ret) {
	//	bool check = false;

	//	if (func_ret->isPointerTy() && callee_ret->isPointerTy()) {
	//		PointerType *pty0 = dyn_cast<PointerType>(func_ret);
	//		PointerType *pty1 = dyn_cast<PointerType>(callee_ret);
	//		Type *ety0 = pty0->getElementType();
	//		Type *ety1 = pty1->getElementType();

	//		if (ety0->isFunctionTy() || ety0->isStructTy()) {
	//			if (pty1 == Type::getInt8PtrTy(*C)) {
	//				errs() << "Found return alias type!\n";
	//				func_ret->dump();
	//				callee_ret->dump();
	//				check = true;
	//			}
	//		}

	//		if (ety1->isFunctionTy() || ety1->isStructTy()) {
	//			if (pty0 == Type::getInt8PtrTy(*C)) {
	//				errs() << "Found return alias type!\n";
	//				func_ret->dump();
	//				callee_ret->dump();
	//				check = true;
	//			}
	//		}
	//	}

	//	// Check type alias of func_ret
	//	if (!type_alias[func_ret]) {
	//		if (auto x = type_alias[func_ret]) {
	//			for (auto y: *x) {
	//				if (y == callee_ret) {
	//					check = true;
	//					break;
	//				}
	//			}
	//		}
	//	}

	//	if (!check)
	//		return false;
	//}

	//errs() << "fty0->dump(): "; fty0->dump();
	//errs() << "fty1->dump(): "; fty1->dump();

	// 4. Check arg type
	unsigned params_num = fty0->getNumParams();

	for (unsigned i=0; i<params_num; i++) {
		bool check = false;
		Type *ty0 = fty0->getParamType(i);
		Type *ty1 = fty1->getParamType(i);

		if (ty0 == ty1) {
			check = true;
    } else if (typeset_map[ty0]) {
      for (auto _ty: *(typeset_map[ty0])) {
        if (_ty == ty1) {
          check = true;
          break;
        }
      }
    }

		if (!check)
			return false;
	}

	return true;
}

void FuncTypeCfgPass::add_typeset(Type *ty0, Type *ty1) {
  auto typeset0 = typeset_map[ty0];
  auto typeset1 = typeset_map[ty1];

  if (typeset0 && typeset1) {
    if (typeset0 != typeset1)
      merge_typeset(ty0, ty1);
  } else if (typeset0) {
    typeset0->insert(ty1);
    typeset_map[ty1] = typeset0;
  } else if (typeset1) {
    typeset1->insert(ty0);
    typeset_map[ty0] = typeset1;
  } else {
    auto new_typeset = new set<Type*>;
    new_typeset->insert(ty0);
    new_typeset->insert(ty1);
    typeset_list.push_back(new_typeset);
    typeset_map[ty0] = new_typeset;
    typeset_map[ty1] = new_typeset;
  }
}

void FuncTypeCfgPass::merge_typeset(Type *ty0, Type *ty1) {
  auto typeset0 = typeset_map[ty0];
  auto typeset1 = typeset_map[ty1];

  for (auto _ty: *typeset1) {
    typeset0->insert(_ty);
    typeset_map[_ty] = typeset0;
  }

  auto it = find(typeset_list.begin(), typeset_list.end(), typeset1);
  ASSERT(it != typeset_list.end());
  typeset_list.erase(it);
}

void FuncTypeCfgPass::handleIndirectCalls(Module &M) {
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

void FuncTypeCfgPass::handleIndirectCall(Module &M, Instruction *pI) {
	FunctionType *fty0 = NULL;
	if (CallInst *pCI = dyn_cast<CallInst>(pI))
		fty0 = pCI->getFunctionType();
	else if (InvokeInst *pII = dyn_cast<InvokeInst>(pI))
		fty0 = pII->getFunctionType();
	else
		ASSERT(false);

	if (fty0->isVarArg()) {
		vector<Type*> indices;

		for (unsigned i = 0; i < pI->getNumOperands()-1; i++) {
			pI->getOperand(i)->dump();
			indices.push_back(pI->getOperand(i)->getType());
		}

		fty0 = FunctionType::get(fty0->getReturnType(), indices, false);
	}

	PointerType *pty0 = PointerType::get(fty0, 0);

  if (typeset_map[pty0])
    return;

  bool found = false;
  for (auto it = typeset_map.begin(); it != typeset_map.end(); it++) {
    PointerType *pty1 = dyn_cast<PointerType>(it->first);
    Type *ety1 = pty1->getElementType();
    if (!pty1 || !ety1->isFunctionTy())
      continue;

    if (typeset_map[pty1]) {
      FunctionType *fty1 = dyn_cast<FunctionType>(ety1);

      if (doTypeComparison(fty0, fty1)) {
        typeset_map[pty1]->insert(pty0);
        typeset_map[pty0] = typeset_map[pty1];
        found = true;
        break;
      }
    }
  }

  if (!found) {
    errs() << "!found pI: "; pI->dump();
  }
}

void FuncTypeCfgPass::init(Module &M) {
	for (auto &F : M) {
		if (&F && F.getName() == "main") {
		  C = &F.getContext();
			DL = &F.getParent()->getDataLayout();
			break;
		}
	}
}

bool FuncTypeCfgPass::isIndirectCall(Instruction *pI) {
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
        if (Function *func = dyn_cast<Function>(bc_op->getOperand(0))) {
          return false;
        }

				if (GlobalAlias *alias = dyn_cast<GlobalAlias>(bc_op->getOperand(0))) {
					return false;
				} 
      }

      return true;
    }

    return false;
  } else if (InvokeInst *pII = dyn_cast<InvokeInst>(pI)) {
    pF = pII->getCalledFunction();

    if (!pF) {
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
          return false;
        }

				if (GlobalAlias *alias = dyn_cast<GlobalAlias>(bc_op->getOperand(0))) {
					return false;
				} 
      }

      return true;
    }
  }

  return false;
}