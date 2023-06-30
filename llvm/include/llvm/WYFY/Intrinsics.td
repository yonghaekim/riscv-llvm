def int_wyfy_tagd : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_i64_ty ], [IntrNoMem]>;
def int_wyfy_tagd_mod : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_i64_ty ], [IntrNoMem]>;
def int_wyfy_xtag : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty ], [IntrNoMem]>;
def int_wyfy_cstr : Intrinsic<[], [ llvm_ptr_ty, llvm_ptr_ty ]>;
def int_wyfy_csrch : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_i64_ty ]>;
//def int_wyfy_bndstr : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_i64_ty ]>;
//def int_wyfy_bndclr : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty ]>;
//def int_wyfy_bndsrch : Intrinsic<[llvm_any_ty], [ llvm_ptr_ty, llvm_i64_ty ]>;
def int_wyfy_fcnt : Intrinsic<[], [ ]>;


//def int_aos_alloc : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_i64_ty ], [IntrNoMem]>;
//def int_aos_dealloc : Intrinsic<[], [ llvm_ptr_ty ]>;
//def int_aos_element : Intrinsic<[], [ llvm_ptr_ty, llvm_i64_ty, llvm_i64_ty ]>;
//def int_aos_element_ret : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_i64_ty, llvm_i64_ty ]>;
////def int_aos_element : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_i64_ty, llvm_i64_ty ]>;
//def int_aos_sign : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_i64_ty, llvm_i64_ty ]>;
////def int_aos_dealloc : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty ], [IntrNoMem]>;
//def int_aos_malloc : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_i64_ty ], [IntrNoMem]>;
//def int_aos_calloc : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_i64_ty ], [IntrNoMem]>;
////def int_aos_realloc : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_i64_ty ], [IntrNoMem]>;
//def int_aos_realloc : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_ptr_ty, llvm_i64_ty ], [IntrNoMem]>;
//def int_aos_free : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty ], [IntrNoMem]>;
////def int_aos_free : Intrinsic<[], [ llvm_ptr_ty ], [IntrNoMem]>;
//def int_aos_autm : Intrinsic<[llvm_anyptr_ty], [ LLVMMatchType<0> ], [IntrNoMem]>;
////def int_aos_autm : Intrinsic<[llvm_anyptr_ty], [ LLVMMatchType<0> ], [IntrNoMem]>;
////def int_aos_autm : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty ], [IntrNoMem]>;
