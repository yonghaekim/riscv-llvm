def int_dpt_tagd : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty ], [IntrNoMem]>;
def int_dpt_tagd_mod : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_i64_ty ], [IntrNoMem]>;
def int_dpt_xtag : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty ], [IntrNoMem]>;
def int_dpt_cstr : Intrinsic<[], [ llvm_ptr_ty, llvm_i64_ty ]>;
def int_dpt_cclr : Intrinsic<[], [ llvm_ptr_ty ]>;
