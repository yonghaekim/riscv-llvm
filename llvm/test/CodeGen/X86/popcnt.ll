; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-unknown | FileCheck %s --check-prefixes=X32,X32-NOSSE
; RUN: llc < %s -mtriple=x86_64-unknown | FileCheck %s --check-prefix=X64
; RUN: llc < %s -mtriple=i686-unknown -mattr=+popcnt | FileCheck %s --check-prefix=X32-POPCNT
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+popcnt | FileCheck %s --check-prefix=X64-POPCNT
; RUN: llc < %s -mtriple=i686-unknown -mattr=sse2 | FileCheck %s --check-prefixes=X32,X32-SSE2
; RUN: llc < %s -mtriple=i686-unknown -mattr=ssse3 | FileCheck %s --check-prefixes=X32,X32-SSSE3

define i8 @cnt8(i8 %x) nounwind readnone {
; X32-LABEL: cnt8:
; X32:       # %bb.0:
; X32-NEXT:    movb {{[0-9]+}}(%esp), %cl
; X32-NEXT:    movl %ecx, %eax
; X32-NEXT:    shrb %al
; X32-NEXT:    andb $85, %al
; X32-NEXT:    subb %al, %cl
; X32-NEXT:    movl %ecx, %eax
; X32-NEXT:    andb $51, %al
; X32-NEXT:    shrb $2, %cl
; X32-NEXT:    andb $51, %cl
; X32-NEXT:    addb %al, %cl
; X32-NEXT:    movl %ecx, %eax
; X32-NEXT:    shrb $4, %al
; X32-NEXT:    addb %cl, %al
; X32-NEXT:    andb $15, %al
; X32-NEXT:    retl
;
; X64-LABEL: cnt8:
; X64:       # %bb.0:
; X64-NEXT:    # kill: def $edi killed $edi def $rdi
; X64-NEXT:    movl %edi, %eax
; X64-NEXT:    shrb %al
; X64-NEXT:    andb $85, %al
; X64-NEXT:    subb %al, %dil
; X64-NEXT:    movl %edi, %eax
; X64-NEXT:    andb $51, %al
; X64-NEXT:    shrb $2, %dil
; X64-NEXT:    andb $51, %dil
; X64-NEXT:    addb %al, %dil
; X64-NEXT:    movl %edi, %eax
; X64-NEXT:    shrb $4, %al
; X64-NEXT:    addl %edi, %eax
; X64-NEXT:    andb $15, %al
; X64-NEXT:    # kill: def $al killed $al killed $eax
; X64-NEXT:    retq
;
; X32-POPCNT-LABEL: cnt8:
; X32-POPCNT:       # %bb.0:
; X32-POPCNT-NEXT:    movzbl {{[0-9]+}}(%esp), %eax
; X32-POPCNT-NEXT:    popcntl %eax, %eax
; X32-POPCNT-NEXT:    # kill: def $al killed $al killed $eax
; X32-POPCNT-NEXT:    retl
;
; X64-POPCNT-LABEL: cnt8:
; X64-POPCNT:       # %bb.0:
; X64-POPCNT-NEXT:    movzbl %dil, %eax
; X64-POPCNT-NEXT:    popcntl %eax, %eax
; X64-POPCNT-NEXT:    # kill: def $al killed $al killed $eax
; X64-POPCNT-NEXT:    retq
  %cnt = tail call i8 @llvm.ctpop.i8(i8 %x)
  ret i8 %cnt
}

define i16 @cnt16(i16 %x) nounwind readnone {
; X32-LABEL: cnt16:
; X32:       # %bb.0:
; X32-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-NEXT:    movl %eax, %ecx
; X32-NEXT:    shrl %ecx
; X32-NEXT:    andl $21845, %ecx # imm = 0x5555
; X32-NEXT:    subl %ecx, %eax
; X32-NEXT:    movl %eax, %ecx
; X32-NEXT:    andl $13107, %ecx # imm = 0x3333
; X32-NEXT:    shrl $2, %eax
; X32-NEXT:    andl $13107, %eax # imm = 0x3333
; X32-NEXT:    addl %ecx, %eax
; X32-NEXT:    movl %eax, %ecx
; X32-NEXT:    shrl $4, %ecx
; X32-NEXT:    addl %eax, %ecx
; X32-NEXT:    andl $3855, %ecx # imm = 0xF0F
; X32-NEXT:    movl %ecx, %eax
; X32-NEXT:    shll $8, %eax
; X32-NEXT:    addl %ecx, %eax
; X32-NEXT:    movzbl %ah, %eax
; X32-NEXT:    # kill: def $ax killed $ax killed $eax
; X32-NEXT:    retl
;
; X64-LABEL: cnt16:
; X64:       # %bb.0:
; X64-NEXT:    movl %edi, %eax
; X64-NEXT:    shrl %eax
; X64-NEXT:    andl $21845, %eax # imm = 0x5555
; X64-NEXT:    subl %eax, %edi
; X64-NEXT:    movl %edi, %eax
; X64-NEXT:    andl $13107, %eax # imm = 0x3333
; X64-NEXT:    shrl $2, %edi
; X64-NEXT:    andl $13107, %edi # imm = 0x3333
; X64-NEXT:    addl %eax, %edi
; X64-NEXT:    movl %edi, %eax
; X64-NEXT:    shrl $4, %eax
; X64-NEXT:    addl %edi, %eax
; X64-NEXT:    andl $3855, %eax # imm = 0xF0F
; X64-NEXT:    movl %eax, %ecx
; X64-NEXT:    shll $8, %ecx
; X64-NEXT:    addl %eax, %ecx
; X64-NEXT:    movzbl %ch, %eax
; X64-NEXT:    # kill: def $ax killed $ax killed $eax
; X64-NEXT:    retq
;
; X32-POPCNT-LABEL: cnt16:
; X32-POPCNT:       # %bb.0:
; X32-POPCNT-NEXT:    popcntw {{[0-9]+}}(%esp), %ax
; X32-POPCNT-NEXT:    retl
;
; X64-POPCNT-LABEL: cnt16:
; X64-POPCNT:       # %bb.0:
; X64-POPCNT-NEXT:    popcntw %di, %ax
; X64-POPCNT-NEXT:    retq
  %cnt = tail call i16 @llvm.ctpop.i16(i16 %x)
  ret i16 %cnt
}

define i32 @cnt32(i32 %x) nounwind readnone {
; X32-LABEL: cnt32:
; X32:       # %bb.0:
; X32-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-NEXT:    movl %eax, %ecx
; X32-NEXT:    shrl %ecx
; X32-NEXT:    andl $1431655765, %ecx # imm = 0x55555555
; X32-NEXT:    subl %ecx, %eax
; X32-NEXT:    movl %eax, %ecx
; X32-NEXT:    andl $858993459, %ecx # imm = 0x33333333
; X32-NEXT:    shrl $2, %eax
; X32-NEXT:    andl $858993459, %eax # imm = 0x33333333
; X32-NEXT:    addl %ecx, %eax
; X32-NEXT:    movl %eax, %ecx
; X32-NEXT:    shrl $4, %ecx
; X32-NEXT:    addl %eax, %ecx
; X32-NEXT:    andl $252645135, %ecx # imm = 0xF0F0F0F
; X32-NEXT:    imull $16843009, %ecx, %eax # imm = 0x1010101
; X32-NEXT:    shrl $24, %eax
; X32-NEXT:    retl
;
; X64-LABEL: cnt32:
; X64:       # %bb.0:
; X64-NEXT:    movl %edi, %eax
; X64-NEXT:    shrl %eax
; X64-NEXT:    andl $1431655765, %eax # imm = 0x55555555
; X64-NEXT:    subl %eax, %edi
; X64-NEXT:    movl %edi, %eax
; X64-NEXT:    andl $858993459, %eax # imm = 0x33333333
; X64-NEXT:    shrl $2, %edi
; X64-NEXT:    andl $858993459, %edi # imm = 0x33333333
; X64-NEXT:    addl %eax, %edi
; X64-NEXT:    movl %edi, %eax
; X64-NEXT:    shrl $4, %eax
; X64-NEXT:    addl %edi, %eax
; X64-NEXT:    andl $252645135, %eax # imm = 0xF0F0F0F
; X64-NEXT:    imull $16843009, %eax, %eax # imm = 0x1010101
; X64-NEXT:    shrl $24, %eax
; X64-NEXT:    retq
;
; X32-POPCNT-LABEL: cnt32:
; X32-POPCNT:       # %bb.0:
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %eax
; X32-POPCNT-NEXT:    retl
;
; X64-POPCNT-LABEL: cnt32:
; X64-POPCNT:       # %bb.0:
; X64-POPCNT-NEXT:    popcntl %edi, %eax
; X64-POPCNT-NEXT:    retq
  %cnt = tail call i32 @llvm.ctpop.i32(i32 %x)
  ret i32 %cnt
}

define i64 @cnt64(i64 %x) nounwind readnone {
; X32-NOSSE-LABEL: cnt64:
; X32-NOSSE:       # %bb.0:
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X32-NOSSE-NEXT:    movl %ecx, %edx
; X32-NOSSE-NEXT:    shrl %edx
; X32-NOSSE-NEXT:    andl $1431655765, %edx # imm = 0x55555555
; X32-NOSSE-NEXT:    subl %edx, %ecx
; X32-NOSSE-NEXT:    movl %ecx, %edx
; X32-NOSSE-NEXT:    andl $858993459, %edx # imm = 0x33333333
; X32-NOSSE-NEXT:    shrl $2, %ecx
; X32-NOSSE-NEXT:    andl $858993459, %ecx # imm = 0x33333333
; X32-NOSSE-NEXT:    addl %edx, %ecx
; X32-NOSSE-NEXT:    movl %ecx, %edx
; X32-NOSSE-NEXT:    shrl $4, %edx
; X32-NOSSE-NEXT:    addl %ecx, %edx
; X32-NOSSE-NEXT:    andl $252645135, %edx # imm = 0xF0F0F0F
; X32-NOSSE-NEXT:    imull $16843009, %edx, %ecx # imm = 0x1010101
; X32-NOSSE-NEXT:    shrl $24, %ecx
; X32-NOSSE-NEXT:    movl %eax, %edx
; X32-NOSSE-NEXT:    shrl %edx
; X32-NOSSE-NEXT:    andl $1431655765, %edx # imm = 0x55555555
; X32-NOSSE-NEXT:    subl %edx, %eax
; X32-NOSSE-NEXT:    movl %eax, %edx
; X32-NOSSE-NEXT:    andl $858993459, %edx # imm = 0x33333333
; X32-NOSSE-NEXT:    shrl $2, %eax
; X32-NOSSE-NEXT:    andl $858993459, %eax # imm = 0x33333333
; X32-NOSSE-NEXT:    addl %edx, %eax
; X32-NOSSE-NEXT:    movl %eax, %edx
; X32-NOSSE-NEXT:    shrl $4, %edx
; X32-NOSSE-NEXT:    addl %eax, %edx
; X32-NOSSE-NEXT:    andl $252645135, %edx # imm = 0xF0F0F0F
; X32-NOSSE-NEXT:    imull $16843009, %edx, %eax # imm = 0x1010101
; X32-NOSSE-NEXT:    shrl $24, %eax
; X32-NOSSE-NEXT:    addl %ecx, %eax
; X32-NOSSE-NEXT:    xorl %edx, %edx
; X32-NOSSE-NEXT:    retl
;
; X64-LABEL: cnt64:
; X64:       # %bb.0:
; X64-NEXT:    movq %rdi, %rax
; X64-NEXT:    shrq %rax
; X64-NEXT:    movabsq $6148914691236517205, %rcx # imm = 0x5555555555555555
; X64-NEXT:    andq %rax, %rcx
; X64-NEXT:    subq %rcx, %rdi
; X64-NEXT:    movabsq $3689348814741910323, %rax # imm = 0x3333333333333333
; X64-NEXT:    movq %rdi, %rcx
; X64-NEXT:    andq %rax, %rcx
; X64-NEXT:    shrq $2, %rdi
; X64-NEXT:    andq %rax, %rdi
; X64-NEXT:    addq %rcx, %rdi
; X64-NEXT:    movq %rdi, %rax
; X64-NEXT:    shrq $4, %rax
; X64-NEXT:    addq %rdi, %rax
; X64-NEXT:    movabsq $1085102592571150095, %rcx # imm = 0xF0F0F0F0F0F0F0F
; X64-NEXT:    andq %rax, %rcx
; X64-NEXT:    movabsq $72340172838076673, %rax # imm = 0x101010101010101
; X64-NEXT:    imulq %rcx, %rax
; X64-NEXT:    shrq $56, %rax
; X64-NEXT:    retq
;
; X32-POPCNT-LABEL: cnt64:
; X32-POPCNT:       # %bb.0:
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %ecx
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %eax
; X32-POPCNT-NEXT:    addl %ecx, %eax
; X32-POPCNT-NEXT:    xorl %edx, %edx
; X32-POPCNT-NEXT:    retl
;
; X64-POPCNT-LABEL: cnt64:
; X64-POPCNT:       # %bb.0:
; X64-POPCNT-NEXT:    popcntq %rdi, %rax
; X64-POPCNT-NEXT:    retq
;
; X32-SSE2-LABEL: cnt64:
; X32-SSE2:       # %bb.0:
; X32-SSE2-NEXT:    movq {{.*#+}} xmm0 = mem[0],zero
; X32-SSE2-NEXT:    movdqa %xmm0, %xmm1
; X32-SSE2-NEXT:    psrlw $1, %xmm1
; X32-SSE2-NEXT:    pand {{\.LCPI.*}}, %xmm1
; X32-SSE2-NEXT:    psubb %xmm1, %xmm0
; X32-SSE2-NEXT:    movdqa {{.*#+}} xmm1 = [51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51]
; X32-SSE2-NEXT:    movdqa %xmm0, %xmm2
; X32-SSE2-NEXT:    pand %xmm1, %xmm2
; X32-SSE2-NEXT:    psrlw $2, %xmm0
; X32-SSE2-NEXT:    pand %xmm1, %xmm0
; X32-SSE2-NEXT:    paddb %xmm2, %xmm0
; X32-SSE2-NEXT:    movdqa %xmm0, %xmm1
; X32-SSE2-NEXT:    psrlw $4, %xmm1
; X32-SSE2-NEXT:    paddb %xmm0, %xmm1
; X32-SSE2-NEXT:    pand {{\.LCPI.*}}, %xmm1
; X32-SSE2-NEXT:    pxor %xmm0, %xmm0
; X32-SSE2-NEXT:    psadbw %xmm1, %xmm0
; X32-SSE2-NEXT:    movd %xmm0, %eax
; X32-SSE2-NEXT:    xorl %edx, %edx
; X32-SSE2-NEXT:    retl
;
; X32-SSSE3-LABEL: cnt64:
; X32-SSSE3:       # %bb.0:
; X32-SSSE3-NEXT:    movdqa {{.*#+}} xmm0 = [15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15]
; X32-SSSE3-NEXT:    movq {{.*#+}} xmm1 = mem[0],zero
; X32-SSSE3-NEXT:    movdqa %xmm1, %xmm2
; X32-SSSE3-NEXT:    pand %xmm0, %xmm2
; X32-SSSE3-NEXT:    movdqa {{.*#+}} xmm3 = [0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4]
; X32-SSSE3-NEXT:    movdqa %xmm3, %xmm4
; X32-SSSE3-NEXT:    pshufb %xmm2, %xmm4
; X32-SSSE3-NEXT:    psrlw $4, %xmm1
; X32-SSSE3-NEXT:    pand %xmm0, %xmm1
; X32-SSSE3-NEXT:    pshufb %xmm1, %xmm3
; X32-SSSE3-NEXT:    paddb %xmm4, %xmm3
; X32-SSSE3-NEXT:    pxor %xmm0, %xmm0
; X32-SSSE3-NEXT:    psadbw %xmm3, %xmm0
; X32-SSSE3-NEXT:    movd %xmm0, %eax
; X32-SSSE3-NEXT:    xorl %edx, %edx
; X32-SSSE3-NEXT:    retl
  %cnt = tail call i64 @llvm.ctpop.i64(i64 %x)
  ret i64 %cnt
}

define i128 @cnt128(i128 %x) nounwind readnone {
; X32-NOSSE-LABEL: cnt128:
; X32-NOSSE:       # %bb.0:
; X32-NOSSE-NEXT:    pushl %ebx
; X32-NOSSE-NEXT:    pushl %edi
; X32-NOSSE-NEXT:    pushl %esi
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %edx
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %esi
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %edi
; X32-NOSSE-NEXT:    movl %edi, %ebx
; X32-NOSSE-NEXT:    shrl %ebx
; X32-NOSSE-NEXT:    andl $1431655765, %ebx # imm = 0x55555555
; X32-NOSSE-NEXT:    subl %ebx, %edi
; X32-NOSSE-NEXT:    movl %edi, %ebx
; X32-NOSSE-NEXT:    andl $858993459, %ebx # imm = 0x33333333
; X32-NOSSE-NEXT:    shrl $2, %edi
; X32-NOSSE-NEXT:    andl $858993459, %edi # imm = 0x33333333
; X32-NOSSE-NEXT:    addl %ebx, %edi
; X32-NOSSE-NEXT:    movl %edi, %ebx
; X32-NOSSE-NEXT:    shrl $4, %ebx
; X32-NOSSE-NEXT:    addl %edi, %ebx
; X32-NOSSE-NEXT:    andl $252645135, %ebx # imm = 0xF0F0F0F
; X32-NOSSE-NEXT:    imull $16843009, %ebx, %edi # imm = 0x1010101
; X32-NOSSE-NEXT:    shrl $24, %edi
; X32-NOSSE-NEXT:    movl %esi, %ebx
; X32-NOSSE-NEXT:    shrl %ebx
; X32-NOSSE-NEXT:    andl $1431655765, %ebx # imm = 0x55555555
; X32-NOSSE-NEXT:    subl %ebx, %esi
; X32-NOSSE-NEXT:    movl %esi, %ebx
; X32-NOSSE-NEXT:    andl $858993459, %ebx # imm = 0x33333333
; X32-NOSSE-NEXT:    shrl $2, %esi
; X32-NOSSE-NEXT:    andl $858993459, %esi # imm = 0x33333333
; X32-NOSSE-NEXT:    addl %ebx, %esi
; X32-NOSSE-NEXT:    movl %esi, %ebx
; X32-NOSSE-NEXT:    shrl $4, %ebx
; X32-NOSSE-NEXT:    addl %esi, %ebx
; X32-NOSSE-NEXT:    andl $252645135, %ebx # imm = 0xF0F0F0F
; X32-NOSSE-NEXT:    imull $16843009, %ebx, %esi # imm = 0x1010101
; X32-NOSSE-NEXT:    shrl $24, %esi
; X32-NOSSE-NEXT:    addl %edi, %esi
; X32-NOSSE-NEXT:    movl %edx, %edi
; X32-NOSSE-NEXT:    shrl %edi
; X32-NOSSE-NEXT:    andl $1431655765, %edi # imm = 0x55555555
; X32-NOSSE-NEXT:    subl %edi, %edx
; X32-NOSSE-NEXT:    movl %edx, %edi
; X32-NOSSE-NEXT:    andl $858993459, %edi # imm = 0x33333333
; X32-NOSSE-NEXT:    shrl $2, %edx
; X32-NOSSE-NEXT:    andl $858993459, %edx # imm = 0x33333333
; X32-NOSSE-NEXT:    addl %edi, %edx
; X32-NOSSE-NEXT:    movl %edx, %edi
; X32-NOSSE-NEXT:    shrl $4, %edi
; X32-NOSSE-NEXT:    addl %edx, %edi
; X32-NOSSE-NEXT:    andl $252645135, %edi # imm = 0xF0F0F0F
; X32-NOSSE-NEXT:    imull $16843009, %edi, %edx # imm = 0x1010101
; X32-NOSSE-NEXT:    shrl $24, %edx
; X32-NOSSE-NEXT:    movl %ecx, %edi
; X32-NOSSE-NEXT:    shrl %edi
; X32-NOSSE-NEXT:    andl $1431655765, %edi # imm = 0x55555555
; X32-NOSSE-NEXT:    subl %edi, %ecx
; X32-NOSSE-NEXT:    movl %ecx, %edi
; X32-NOSSE-NEXT:    andl $858993459, %edi # imm = 0x33333333
; X32-NOSSE-NEXT:    shrl $2, %ecx
; X32-NOSSE-NEXT:    andl $858993459, %ecx # imm = 0x33333333
; X32-NOSSE-NEXT:    addl %edi, %ecx
; X32-NOSSE-NEXT:    movl %ecx, %edi
; X32-NOSSE-NEXT:    shrl $4, %edi
; X32-NOSSE-NEXT:    addl %ecx, %edi
; X32-NOSSE-NEXT:    andl $252645135, %edi # imm = 0xF0F0F0F
; X32-NOSSE-NEXT:    imull $16843009, %edi, %ecx # imm = 0x1010101
; X32-NOSSE-NEXT:    shrl $24, %ecx
; X32-NOSSE-NEXT:    addl %edx, %ecx
; X32-NOSSE-NEXT:    addl %esi, %ecx
; X32-NOSSE-NEXT:    movl %ecx, (%eax)
; X32-NOSSE-NEXT:    movl $0, 12(%eax)
; X32-NOSSE-NEXT:    movl $0, 8(%eax)
; X32-NOSSE-NEXT:    movl $0, 4(%eax)
; X32-NOSSE-NEXT:    popl %esi
; X32-NOSSE-NEXT:    popl %edi
; X32-NOSSE-NEXT:    popl %ebx
; X32-NOSSE-NEXT:    retl $4
;
; X64-LABEL: cnt128:
; X64:       # %bb.0:
; X64-NEXT:    movq %rsi, %rax
; X64-NEXT:    shrq %rax
; X64-NEXT:    movabsq $6148914691236517205, %r8 # imm = 0x5555555555555555
; X64-NEXT:    andq %r8, %rax
; X64-NEXT:    subq %rax, %rsi
; X64-NEXT:    movabsq $3689348814741910323, %rax # imm = 0x3333333333333333
; X64-NEXT:    movq %rsi, %rcx
; X64-NEXT:    andq %rax, %rcx
; X64-NEXT:    shrq $2, %rsi
; X64-NEXT:    andq %rax, %rsi
; X64-NEXT:    addq %rcx, %rsi
; X64-NEXT:    movq %rsi, %rcx
; X64-NEXT:    shrq $4, %rcx
; X64-NEXT:    addq %rsi, %rcx
; X64-NEXT:    movabsq $1085102592571150095, %r9 # imm = 0xF0F0F0F0F0F0F0F
; X64-NEXT:    andq %r9, %rcx
; X64-NEXT:    movabsq $72340172838076673, %rdx # imm = 0x101010101010101
; X64-NEXT:    imulq %rdx, %rcx
; X64-NEXT:    shrq $56, %rcx
; X64-NEXT:    movq %rdi, %rsi
; X64-NEXT:    shrq %rsi
; X64-NEXT:    andq %r8, %rsi
; X64-NEXT:    subq %rsi, %rdi
; X64-NEXT:    movq %rdi, %rsi
; X64-NEXT:    andq %rax, %rsi
; X64-NEXT:    shrq $2, %rdi
; X64-NEXT:    andq %rax, %rdi
; X64-NEXT:    addq %rsi, %rdi
; X64-NEXT:    movq %rdi, %rax
; X64-NEXT:    shrq $4, %rax
; X64-NEXT:    addq %rdi, %rax
; X64-NEXT:    andq %r9, %rax
; X64-NEXT:    imulq %rdx, %rax
; X64-NEXT:    shrq $56, %rax
; X64-NEXT:    addq %rcx, %rax
; X64-NEXT:    xorl %edx, %edx
; X64-NEXT:    retq
;
; X32-POPCNT-LABEL: cnt128:
; X32-POPCNT:       # %bb.0:
; X32-POPCNT-NEXT:    pushl %esi
; X32-POPCNT-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %ecx
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %edx
; X32-POPCNT-NEXT:    addl %ecx, %edx
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %ecx
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %esi
; X32-POPCNT-NEXT:    addl %ecx, %esi
; X32-POPCNT-NEXT:    addl %edx, %esi
; X32-POPCNT-NEXT:    movl %esi, (%eax)
; X32-POPCNT-NEXT:    movl $0, 12(%eax)
; X32-POPCNT-NEXT:    movl $0, 8(%eax)
; X32-POPCNT-NEXT:    movl $0, 4(%eax)
; X32-POPCNT-NEXT:    popl %esi
; X32-POPCNT-NEXT:    retl $4
;
; X64-POPCNT-LABEL: cnt128:
; X64-POPCNT:       # %bb.0:
; X64-POPCNT-NEXT:    popcntq %rsi, %rcx
; X64-POPCNT-NEXT:    popcntq %rdi, %rax
; X64-POPCNT-NEXT:    addq %rcx, %rax
; X64-POPCNT-NEXT:    xorl %edx, %edx
; X64-POPCNT-NEXT:    retq
;
; X32-SSE2-LABEL: cnt128:
; X32-SSE2:       # %bb.0:
; X32-SSE2-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-SSE2-NEXT:    movq {{.*#+}} xmm0 = mem[0],zero
; X32-SSE2-NEXT:    movdqa %xmm0, %xmm1
; X32-SSE2-NEXT:    psrlw $1, %xmm1
; X32-SSE2-NEXT:    movdqa {{.*#+}} xmm2 = [85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85]
; X32-SSE2-NEXT:    pand %xmm2, %xmm1
; X32-SSE2-NEXT:    psubb %xmm1, %xmm0
; X32-SSE2-NEXT:    movdqa {{.*#+}} xmm1 = [51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51]
; X32-SSE2-NEXT:    movdqa %xmm0, %xmm3
; X32-SSE2-NEXT:    pand %xmm1, %xmm3
; X32-SSE2-NEXT:    psrlw $2, %xmm0
; X32-SSE2-NEXT:    pand %xmm1, %xmm0
; X32-SSE2-NEXT:    paddb %xmm3, %xmm0
; X32-SSE2-NEXT:    movdqa %xmm0, %xmm3
; X32-SSE2-NEXT:    psrlw $4, %xmm3
; X32-SSE2-NEXT:    paddb %xmm0, %xmm3
; X32-SSE2-NEXT:    movdqa {{.*#+}} xmm0 = [15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15]
; X32-SSE2-NEXT:    pand %xmm0, %xmm3
; X32-SSE2-NEXT:    pxor %xmm4, %xmm4
; X32-SSE2-NEXT:    psadbw %xmm4, %xmm3
; X32-SSE2-NEXT:    movd %xmm3, %ecx
; X32-SSE2-NEXT:    movq {{.*#+}} xmm3 = mem[0],zero
; X32-SSE2-NEXT:    movdqa %xmm3, %xmm5
; X32-SSE2-NEXT:    psrlw $1, %xmm5
; X32-SSE2-NEXT:    pand %xmm2, %xmm5
; X32-SSE2-NEXT:    psubb %xmm5, %xmm3
; X32-SSE2-NEXT:    movdqa %xmm3, %xmm2
; X32-SSE2-NEXT:    pand %xmm1, %xmm2
; X32-SSE2-NEXT:    psrlw $2, %xmm3
; X32-SSE2-NEXT:    pand %xmm1, %xmm3
; X32-SSE2-NEXT:    paddb %xmm2, %xmm3
; X32-SSE2-NEXT:    movdqa %xmm3, %xmm1
; X32-SSE2-NEXT:    psrlw $4, %xmm1
; X32-SSE2-NEXT:    paddb %xmm3, %xmm1
; X32-SSE2-NEXT:    pand %xmm0, %xmm1
; X32-SSE2-NEXT:    psadbw %xmm4, %xmm1
; X32-SSE2-NEXT:    movd %xmm1, %edx
; X32-SSE2-NEXT:    addl %ecx, %edx
; X32-SSE2-NEXT:    movl %edx, (%eax)
; X32-SSE2-NEXT:    movl $0, 12(%eax)
; X32-SSE2-NEXT:    movl $0, 8(%eax)
; X32-SSE2-NEXT:    movl $0, 4(%eax)
; X32-SSE2-NEXT:    retl $4
;
; X32-SSSE3-LABEL: cnt128:
; X32-SSSE3:       # %bb.0:
; X32-SSSE3-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-SSSE3-NEXT:    movdqa {{.*#+}} xmm0 = [15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15]
; X32-SSSE3-NEXT:    movq {{.*#+}} xmm1 = mem[0],zero
; X32-SSSE3-NEXT:    movdqa %xmm1, %xmm2
; X32-SSSE3-NEXT:    pand %xmm0, %xmm2
; X32-SSSE3-NEXT:    movdqa {{.*#+}} xmm3 = [0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4]
; X32-SSSE3-NEXT:    movdqa %xmm3, %xmm4
; X32-SSSE3-NEXT:    pshufb %xmm2, %xmm4
; X32-SSSE3-NEXT:    psrlw $4, %xmm1
; X32-SSSE3-NEXT:    pand %xmm0, %xmm1
; X32-SSSE3-NEXT:    movdqa %xmm3, %xmm2
; X32-SSSE3-NEXT:    pshufb %xmm1, %xmm2
; X32-SSSE3-NEXT:    paddb %xmm4, %xmm2
; X32-SSSE3-NEXT:    pxor %xmm1, %xmm1
; X32-SSSE3-NEXT:    psadbw %xmm1, %xmm2
; X32-SSSE3-NEXT:    movd %xmm2, %ecx
; X32-SSSE3-NEXT:    movq {{.*#+}} xmm2 = mem[0],zero
; X32-SSSE3-NEXT:    movdqa %xmm2, %xmm4
; X32-SSSE3-NEXT:    pand %xmm0, %xmm4
; X32-SSSE3-NEXT:    movdqa %xmm3, %xmm5
; X32-SSSE3-NEXT:    pshufb %xmm4, %xmm5
; X32-SSSE3-NEXT:    psrlw $4, %xmm2
; X32-SSSE3-NEXT:    pand %xmm0, %xmm2
; X32-SSSE3-NEXT:    pshufb %xmm2, %xmm3
; X32-SSSE3-NEXT:    paddb %xmm5, %xmm3
; X32-SSSE3-NEXT:    psadbw %xmm1, %xmm3
; X32-SSSE3-NEXT:    movd %xmm3, %edx
; X32-SSSE3-NEXT:    addl %ecx, %edx
; X32-SSSE3-NEXT:    movl %edx, (%eax)
; X32-SSSE3-NEXT:    movl $0, 12(%eax)
; X32-SSSE3-NEXT:    movl $0, 8(%eax)
; X32-SSSE3-NEXT:    movl $0, 4(%eax)
; X32-SSSE3-NEXT:    retl $4
  %cnt = tail call i128 @llvm.ctpop.i128(i128 %x)
  ret i128 %cnt
}

define i64 @cnt64_noimplicitfloat(i64 %x) nounwind readnone noimplicitfloat  {
; X32-LABEL: cnt64_noimplicitfloat:
; X32:       # %bb.0:
; X32-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X32-NEXT:    movl %ecx, %edx
; X32-NEXT:    shrl %edx
; X32-NEXT:    andl $1431655765, %edx # imm = 0x55555555
; X32-NEXT:    subl %edx, %ecx
; X32-NEXT:    movl %ecx, %edx
; X32-NEXT:    andl $858993459, %edx # imm = 0x33333333
; X32-NEXT:    shrl $2, %ecx
; X32-NEXT:    andl $858993459, %ecx # imm = 0x33333333
; X32-NEXT:    addl %edx, %ecx
; X32-NEXT:    movl %ecx, %edx
; X32-NEXT:    shrl $4, %edx
; X32-NEXT:    addl %ecx, %edx
; X32-NEXT:    andl $252645135, %edx # imm = 0xF0F0F0F
; X32-NEXT:    imull $16843009, %edx, %ecx # imm = 0x1010101
; X32-NEXT:    shrl $24, %ecx
; X32-NEXT:    movl %eax, %edx
; X32-NEXT:    shrl %edx
; X32-NEXT:    andl $1431655765, %edx # imm = 0x55555555
; X32-NEXT:    subl %edx, %eax
; X32-NEXT:    movl %eax, %edx
; X32-NEXT:    andl $858993459, %edx # imm = 0x33333333
; X32-NEXT:    shrl $2, %eax
; X32-NEXT:    andl $858993459, %eax # imm = 0x33333333
; X32-NEXT:    addl %edx, %eax
; X32-NEXT:    movl %eax, %edx
; X32-NEXT:    shrl $4, %edx
; X32-NEXT:    addl %eax, %edx
; X32-NEXT:    andl $252645135, %edx # imm = 0xF0F0F0F
; X32-NEXT:    imull $16843009, %edx, %eax # imm = 0x1010101
; X32-NEXT:    shrl $24, %eax
; X32-NEXT:    addl %ecx, %eax
; X32-NEXT:    xorl %edx, %edx
; X32-NEXT:    retl
;
; X64-LABEL: cnt64_noimplicitfloat:
; X64:       # %bb.0:
; X64-NEXT:    movq %rdi, %rax
; X64-NEXT:    shrq %rax
; X64-NEXT:    movabsq $6148914691236517205, %rcx # imm = 0x5555555555555555
; X64-NEXT:    andq %rax, %rcx
; X64-NEXT:    subq %rcx, %rdi
; X64-NEXT:    movabsq $3689348814741910323, %rax # imm = 0x3333333333333333
; X64-NEXT:    movq %rdi, %rcx
; X64-NEXT:    andq %rax, %rcx
; X64-NEXT:    shrq $2, %rdi
; X64-NEXT:    andq %rax, %rdi
; X64-NEXT:    addq %rcx, %rdi
; X64-NEXT:    movq %rdi, %rax
; X64-NEXT:    shrq $4, %rax
; X64-NEXT:    addq %rdi, %rax
; X64-NEXT:    movabsq $1085102592571150095, %rcx # imm = 0xF0F0F0F0F0F0F0F
; X64-NEXT:    andq %rax, %rcx
; X64-NEXT:    movabsq $72340172838076673, %rax # imm = 0x101010101010101
; X64-NEXT:    imulq %rcx, %rax
; X64-NEXT:    shrq $56, %rax
; X64-NEXT:    retq
;
; X32-POPCNT-LABEL: cnt64_noimplicitfloat:
; X32-POPCNT:       # %bb.0:
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %ecx
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %eax
; X32-POPCNT-NEXT:    addl %ecx, %eax
; X32-POPCNT-NEXT:    xorl %edx, %edx
; X32-POPCNT-NEXT:    retl
;
; X64-POPCNT-LABEL: cnt64_noimplicitfloat:
; X64-POPCNT:       # %bb.0:
; X64-POPCNT-NEXT:    popcntq %rdi, %rax
; X64-POPCNT-NEXT:    retq
  %cnt = tail call i64 @llvm.ctpop.i64(i64 %x)
  ret i64 %cnt
}

define i32 @cnt32_optsize(i32 %x) nounwind readnone optsize {
; X32-LABEL: cnt32_optsize:
; X32:       # %bb.0:
; X32-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-NEXT:    movl %eax, %ecx
; X32-NEXT:    shrl %ecx
; X32-NEXT:    andl $1431655765, %ecx # imm = 0x55555555
; X32-NEXT:    subl %ecx, %eax
; X32-NEXT:    movl $858993459, %ecx # imm = 0x33333333
; X32-NEXT:    movl %eax, %edx
; X32-NEXT:    andl %ecx, %edx
; X32-NEXT:    shrl $2, %eax
; X32-NEXT:    andl %ecx, %eax
; X32-NEXT:    addl %edx, %eax
; X32-NEXT:    movl %eax, %ecx
; X32-NEXT:    shrl $4, %ecx
; X32-NEXT:    addl %eax, %ecx
; X32-NEXT:    andl $252645135, %ecx # imm = 0xF0F0F0F
; X32-NEXT:    imull $16843009, %ecx, %eax # imm = 0x1010101
; X32-NEXT:    shrl $24, %eax
; X32-NEXT:    retl
;
; X64-LABEL: cnt32_optsize:
; X64:       # %bb.0:
; X64-NEXT:    movl %edi, %eax
; X64-NEXT:    shrl %eax
; X64-NEXT:    andl $1431655765, %eax # imm = 0x55555555
; X64-NEXT:    subl %eax, %edi
; X64-NEXT:    movl $858993459, %eax # imm = 0x33333333
; X64-NEXT:    movl %edi, %ecx
; X64-NEXT:    andl %eax, %ecx
; X64-NEXT:    shrl $2, %edi
; X64-NEXT:    andl %eax, %edi
; X64-NEXT:    addl %ecx, %edi
; X64-NEXT:    movl %edi, %eax
; X64-NEXT:    shrl $4, %eax
; X64-NEXT:    addl %edi, %eax
; X64-NEXT:    andl $252645135, %eax # imm = 0xF0F0F0F
; X64-NEXT:    imull $16843009, %eax, %eax # imm = 0x1010101
; X64-NEXT:    shrl $24, %eax
; X64-NEXT:    retq
;
; X32-POPCNT-LABEL: cnt32_optsize:
; X32-POPCNT:       # %bb.0:
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %eax
; X32-POPCNT-NEXT:    retl
;
; X64-POPCNT-LABEL: cnt32_optsize:
; X64-POPCNT:       # %bb.0:
; X64-POPCNT-NEXT:    popcntl %edi, %eax
; X64-POPCNT-NEXT:    retq
  %cnt = tail call i32 @llvm.ctpop.i32(i32 %x)
  ret i32 %cnt
}

define i64 @cnt64_optsize(i64 %x) nounwind readnone optsize {
; X32-NOSSE-LABEL: cnt64_optsize:
; X32-NOSSE:       # %bb.0:
; X32-NOSSE-NEXT:    pushl %ebx
; X32-NOSSE-NEXT:    pushl %edi
; X32-NOSSE-NEXT:    pushl %esi
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %ecx
; X32-NOSSE-NEXT:    movl %ecx, %edx
; X32-NOSSE-NEXT:    shrl %edx
; X32-NOSSE-NEXT:    movl $1431655765, %esi # imm = 0x55555555
; X32-NOSSE-NEXT:    andl %esi, %edx
; X32-NOSSE-NEXT:    subl %edx, %ecx
; X32-NOSSE-NEXT:    movl $858993459, %edx # imm = 0x33333333
; X32-NOSSE-NEXT:    movl %ecx, %edi
; X32-NOSSE-NEXT:    andl %edx, %edi
; X32-NOSSE-NEXT:    shrl $2, %ecx
; X32-NOSSE-NEXT:    andl %edx, %ecx
; X32-NOSSE-NEXT:    addl %edi, %ecx
; X32-NOSSE-NEXT:    movl %ecx, %edi
; X32-NOSSE-NEXT:    shrl $4, %edi
; X32-NOSSE-NEXT:    addl %ecx, %edi
; X32-NOSSE-NEXT:    movl $252645135, %ecx # imm = 0xF0F0F0F
; X32-NOSSE-NEXT:    andl %ecx, %edi
; X32-NOSSE-NEXT:    imull $16843009, %edi, %edi # imm = 0x1010101
; X32-NOSSE-NEXT:    shrl $24, %edi
; X32-NOSSE-NEXT:    movl %eax, %ebx
; X32-NOSSE-NEXT:    shrl %ebx
; X32-NOSSE-NEXT:    andl %esi, %ebx
; X32-NOSSE-NEXT:    subl %ebx, %eax
; X32-NOSSE-NEXT:    movl %eax, %esi
; X32-NOSSE-NEXT:    andl %edx, %esi
; X32-NOSSE-NEXT:    shrl $2, %eax
; X32-NOSSE-NEXT:    andl %edx, %eax
; X32-NOSSE-NEXT:    addl %esi, %eax
; X32-NOSSE-NEXT:    movl %eax, %edx
; X32-NOSSE-NEXT:    shrl $4, %edx
; X32-NOSSE-NEXT:    addl %eax, %edx
; X32-NOSSE-NEXT:    andl %ecx, %edx
; X32-NOSSE-NEXT:    imull $16843009, %edx, %eax # imm = 0x1010101
; X32-NOSSE-NEXT:    shrl $24, %eax
; X32-NOSSE-NEXT:    addl %edi, %eax
; X32-NOSSE-NEXT:    xorl %edx, %edx
; X32-NOSSE-NEXT:    popl %esi
; X32-NOSSE-NEXT:    popl %edi
; X32-NOSSE-NEXT:    popl %ebx
; X32-NOSSE-NEXT:    retl
;
; X64-LABEL: cnt64_optsize:
; X64:       # %bb.0:
; X64-NEXT:    movq %rdi, %rax
; X64-NEXT:    shrq %rax
; X64-NEXT:    movabsq $6148914691236517205, %rcx # imm = 0x5555555555555555
; X64-NEXT:    andq %rax, %rcx
; X64-NEXT:    subq %rcx, %rdi
; X64-NEXT:    movabsq $3689348814741910323, %rax # imm = 0x3333333333333333
; X64-NEXT:    movq %rdi, %rcx
; X64-NEXT:    andq %rax, %rcx
; X64-NEXT:    shrq $2, %rdi
; X64-NEXT:    andq %rax, %rdi
; X64-NEXT:    addq %rcx, %rdi
; X64-NEXT:    movq %rdi, %rax
; X64-NEXT:    shrq $4, %rax
; X64-NEXT:    addq %rdi, %rax
; X64-NEXT:    movabsq $1085102592571150095, %rcx # imm = 0xF0F0F0F0F0F0F0F
; X64-NEXT:    andq %rax, %rcx
; X64-NEXT:    movabsq $72340172838076673, %rax # imm = 0x101010101010101
; X64-NEXT:    imulq %rcx, %rax
; X64-NEXT:    shrq $56, %rax
; X64-NEXT:    retq
;
; X32-POPCNT-LABEL: cnt64_optsize:
; X32-POPCNT:       # %bb.0:
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %ecx
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %eax
; X32-POPCNT-NEXT:    addl %ecx, %eax
; X32-POPCNT-NEXT:    xorl %edx, %edx
; X32-POPCNT-NEXT:    retl
;
; X64-POPCNT-LABEL: cnt64_optsize:
; X64-POPCNT:       # %bb.0:
; X64-POPCNT-NEXT:    popcntq %rdi, %rax
; X64-POPCNT-NEXT:    retq
;
; X32-SSE2-LABEL: cnt64_optsize:
; X32-SSE2:       # %bb.0:
; X32-SSE2-NEXT:    movq {{.*#+}} xmm0 = mem[0],zero
; X32-SSE2-NEXT:    movdqa %xmm0, %xmm1
; X32-SSE2-NEXT:    psrlw $1, %xmm1
; X32-SSE2-NEXT:    pand {{\.LCPI.*}}, %xmm1
; X32-SSE2-NEXT:    psubb %xmm1, %xmm0
; X32-SSE2-NEXT:    movdqa {{.*#+}} xmm1 = [51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51]
; X32-SSE2-NEXT:    movdqa %xmm0, %xmm2
; X32-SSE2-NEXT:    pand %xmm1, %xmm2
; X32-SSE2-NEXT:    psrlw $2, %xmm0
; X32-SSE2-NEXT:    pand %xmm1, %xmm0
; X32-SSE2-NEXT:    paddb %xmm2, %xmm0
; X32-SSE2-NEXT:    movdqa %xmm0, %xmm1
; X32-SSE2-NEXT:    psrlw $4, %xmm1
; X32-SSE2-NEXT:    paddb %xmm0, %xmm1
; X32-SSE2-NEXT:    pand {{\.LCPI.*}}, %xmm1
; X32-SSE2-NEXT:    pxor %xmm0, %xmm0
; X32-SSE2-NEXT:    psadbw %xmm1, %xmm0
; X32-SSE2-NEXT:    movd %xmm0, %eax
; X32-SSE2-NEXT:    xorl %edx, %edx
; X32-SSE2-NEXT:    retl
;
; X32-SSSE3-LABEL: cnt64_optsize:
; X32-SSSE3:       # %bb.0:
; X32-SSSE3-NEXT:    movdqa {{.*#+}} xmm0 = [15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15]
; X32-SSSE3-NEXT:    movq {{.*#+}} xmm1 = mem[0],zero
; X32-SSSE3-NEXT:    movdqa %xmm1, %xmm2
; X32-SSSE3-NEXT:    pand %xmm0, %xmm2
; X32-SSSE3-NEXT:    movdqa {{.*#+}} xmm3 = [0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4]
; X32-SSSE3-NEXT:    movdqa %xmm3, %xmm4
; X32-SSSE3-NEXT:    pshufb %xmm2, %xmm4
; X32-SSSE3-NEXT:    psrlw $4, %xmm1
; X32-SSSE3-NEXT:    pand %xmm0, %xmm1
; X32-SSSE3-NEXT:    pshufb %xmm1, %xmm3
; X32-SSSE3-NEXT:    paddb %xmm4, %xmm3
; X32-SSSE3-NEXT:    pxor %xmm0, %xmm0
; X32-SSSE3-NEXT:    psadbw %xmm3, %xmm0
; X32-SSSE3-NEXT:    movd %xmm0, %eax
; X32-SSSE3-NEXT:    xorl %edx, %edx
; X32-SSSE3-NEXT:    retl
  %cnt = tail call i64 @llvm.ctpop.i64(i64 %x)
  ret i64 %cnt
}

define i128 @cnt128_optsize(i128 %x) nounwind readnone optsize {
; X32-NOSSE-LABEL: cnt128_optsize:
; X32-NOSSE:       # %bb.0:
; X32-NOSSE-NEXT:    pushl %ebp
; X32-NOSSE-NEXT:    pushl %ebx
; X32-NOSSE-NEXT:    pushl %edi
; X32-NOSSE-NEXT:    pushl %esi
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %edx
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %esi
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %ebx
; X32-NOSSE-NEXT:    movl %ebx, %ecx
; X32-NOSSE-NEXT:    shrl %ecx
; X32-NOSSE-NEXT:    movl $1431655765, %edi # imm = 0x55555555
; X32-NOSSE-NEXT:    andl %edi, %ecx
; X32-NOSSE-NEXT:    movl $1431655765, %edi # imm = 0x55555555
; X32-NOSSE-NEXT:    subl %ecx, %ebx
; X32-NOSSE-NEXT:    movl $858993459, %ecx # imm = 0x33333333
; X32-NOSSE-NEXT:    movl %ebx, %ebp
; X32-NOSSE-NEXT:    andl %ecx, %ebp
; X32-NOSSE-NEXT:    shrl $2, %ebx
; X32-NOSSE-NEXT:    andl %ecx, %ebx
; X32-NOSSE-NEXT:    addl %ebp, %ebx
; X32-NOSSE-NEXT:    movl %ebx, %ebp
; X32-NOSSE-NEXT:    shrl $4, %ebp
; X32-NOSSE-NEXT:    addl %ebx, %ebp
; X32-NOSSE-NEXT:    movl %eax, %ebx
; X32-NOSSE-NEXT:    shrl %ebx
; X32-NOSSE-NEXT:    andl %edi, %ebx
; X32-NOSSE-NEXT:    subl %ebx, %eax
; X32-NOSSE-NEXT:    movl %eax, %ebx
; X32-NOSSE-NEXT:    andl %ecx, %ebx
; X32-NOSSE-NEXT:    shrl $2, %eax
; X32-NOSSE-NEXT:    andl %ecx, %eax
; X32-NOSSE-NEXT:    addl %ebx, %eax
; X32-NOSSE-NEXT:    movl %eax, %edi
; X32-NOSSE-NEXT:    shrl $4, %edi
; X32-NOSSE-NEXT:    addl %eax, %edi
; X32-NOSSE-NEXT:    movl $252645135, %ebx # imm = 0xF0F0F0F
; X32-NOSSE-NEXT:    andl %ebx, %ebp
; X32-NOSSE-NEXT:    imull $16843009, %ebp, %eax # imm = 0x1010101
; X32-NOSSE-NEXT:    shrl $24, %eax
; X32-NOSSE-NEXT:    andl %ebx, %edi
; X32-NOSSE-NEXT:    imull $16843009, %edi, %edi # imm = 0x1010101
; X32-NOSSE-NEXT:    shrl $24, %edi
; X32-NOSSE-NEXT:    addl %eax, %edi
; X32-NOSSE-NEXT:    movl %esi, %eax
; X32-NOSSE-NEXT:    shrl %eax
; X32-NOSSE-NEXT:    movl $1431655765, %ebp # imm = 0x55555555
; X32-NOSSE-NEXT:    andl %ebp, %eax
; X32-NOSSE-NEXT:    subl %eax, %esi
; X32-NOSSE-NEXT:    movl %esi, %eax
; X32-NOSSE-NEXT:    andl %ecx, %eax
; X32-NOSSE-NEXT:    shrl $2, %esi
; X32-NOSSE-NEXT:    andl %ecx, %esi
; X32-NOSSE-NEXT:    addl %eax, %esi
; X32-NOSSE-NEXT:    movl %esi, %eax
; X32-NOSSE-NEXT:    shrl $4, %eax
; X32-NOSSE-NEXT:    addl %esi, %eax
; X32-NOSSE-NEXT:    movl %edx, %esi
; X32-NOSSE-NEXT:    shrl %esi
; X32-NOSSE-NEXT:    andl %ebp, %esi
; X32-NOSSE-NEXT:    subl %esi, %edx
; X32-NOSSE-NEXT:    movl %edx, %esi
; X32-NOSSE-NEXT:    andl %ecx, %esi
; X32-NOSSE-NEXT:    shrl $2, %edx
; X32-NOSSE-NEXT:    andl %ecx, %edx
; X32-NOSSE-NEXT:    addl %esi, %edx
; X32-NOSSE-NEXT:    movl %edx, %ecx
; X32-NOSSE-NEXT:    shrl $4, %ecx
; X32-NOSSE-NEXT:    addl %edx, %ecx
; X32-NOSSE-NEXT:    andl %ebx, %eax
; X32-NOSSE-NEXT:    andl %ebx, %ecx
; X32-NOSSE-NEXT:    imull $16843009, %eax, %eax # imm = 0x1010101
; X32-NOSSE-NEXT:    shrl $24, %eax
; X32-NOSSE-NEXT:    imull $16843009, %ecx, %ecx # imm = 0x1010101
; X32-NOSSE-NEXT:    shrl $24, %ecx
; X32-NOSSE-NEXT:    addl %eax, %ecx
; X32-NOSSE-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-NOSSE-NEXT:    addl %edi, %ecx
; X32-NOSSE-NEXT:    xorl %edx, %edx
; X32-NOSSE-NEXT:    movl %edx, 12(%eax)
; X32-NOSSE-NEXT:    movl %edx, 8(%eax)
; X32-NOSSE-NEXT:    movl %edx, 4(%eax)
; X32-NOSSE-NEXT:    movl %ecx, (%eax)
; X32-NOSSE-NEXT:    popl %esi
; X32-NOSSE-NEXT:    popl %edi
; X32-NOSSE-NEXT:    popl %ebx
; X32-NOSSE-NEXT:    popl %ebp
; X32-NOSSE-NEXT:    retl $4
;
; X64-LABEL: cnt128_optsize:
; X64:       # %bb.0:
; X64-NEXT:    movq %rsi, %rax
; X64-NEXT:    shrq %rax
; X64-NEXT:    movabsq $6148914691236517205, %r8 # imm = 0x5555555555555555
; X64-NEXT:    andq %r8, %rax
; X64-NEXT:    subq %rax, %rsi
; X64-NEXT:    movabsq $3689348814741910323, %rax # imm = 0x3333333333333333
; X64-NEXT:    movq %rsi, %rcx
; X64-NEXT:    andq %rax, %rcx
; X64-NEXT:    shrq $2, %rsi
; X64-NEXT:    andq %rax, %rsi
; X64-NEXT:    addq %rcx, %rsi
; X64-NEXT:    movq %rsi, %rcx
; X64-NEXT:    shrq $4, %rcx
; X64-NEXT:    addq %rsi, %rcx
; X64-NEXT:    movabsq $1085102592571150095, %r9 # imm = 0xF0F0F0F0F0F0F0F
; X64-NEXT:    andq %r9, %rcx
; X64-NEXT:    movabsq $72340172838076673, %rdx # imm = 0x101010101010101
; X64-NEXT:    imulq %rdx, %rcx
; X64-NEXT:    shrq $56, %rcx
; X64-NEXT:    movq %rdi, %rsi
; X64-NEXT:    shrq %rsi
; X64-NEXT:    andq %r8, %rsi
; X64-NEXT:    subq %rsi, %rdi
; X64-NEXT:    movq %rdi, %rsi
; X64-NEXT:    andq %rax, %rsi
; X64-NEXT:    shrq $2, %rdi
; X64-NEXT:    andq %rax, %rdi
; X64-NEXT:    addq %rsi, %rdi
; X64-NEXT:    movq %rdi, %rax
; X64-NEXT:    shrq $4, %rax
; X64-NEXT:    addq %rdi, %rax
; X64-NEXT:    andq %r9, %rax
; X64-NEXT:    imulq %rdx, %rax
; X64-NEXT:    shrq $56, %rax
; X64-NEXT:    addq %rcx, %rax
; X64-NEXT:    xorl %edx, %edx
; X64-NEXT:    retq
;
; X32-POPCNT-LABEL: cnt128_optsize:
; X32-POPCNT:       # %bb.0:
; X32-POPCNT-NEXT:    pushl %esi
; X32-POPCNT-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %ecx
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %edx
; X32-POPCNT-NEXT:    addl %ecx, %edx
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %ecx
; X32-POPCNT-NEXT:    popcntl {{[0-9]+}}(%esp), %esi
; X32-POPCNT-NEXT:    addl %ecx, %esi
; X32-POPCNT-NEXT:    addl %edx, %esi
; X32-POPCNT-NEXT:    xorl %ecx, %ecx
; X32-POPCNT-NEXT:    movl %ecx, 12(%eax)
; X32-POPCNT-NEXT:    movl %ecx, 8(%eax)
; X32-POPCNT-NEXT:    movl %ecx, 4(%eax)
; X32-POPCNT-NEXT:    movl %esi, (%eax)
; X32-POPCNT-NEXT:    popl %esi
; X32-POPCNT-NEXT:    retl $4
;
; X64-POPCNT-LABEL: cnt128_optsize:
; X64-POPCNT:       # %bb.0:
; X64-POPCNT-NEXT:    popcntq %rsi, %rcx
; X64-POPCNT-NEXT:    popcntq %rdi, %rax
; X64-POPCNT-NEXT:    addq %rcx, %rax
; X64-POPCNT-NEXT:    xorl %edx, %edx
; X64-POPCNT-NEXT:    retq
;
; X32-SSE2-LABEL: cnt128_optsize:
; X32-SSE2:       # %bb.0:
; X32-SSE2-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-SSE2-NEXT:    movq {{.*#+}} xmm0 = mem[0],zero
; X32-SSE2-NEXT:    movdqa %xmm0, %xmm1
; X32-SSE2-NEXT:    psrlw $1, %xmm1
; X32-SSE2-NEXT:    movdqa {{.*#+}} xmm2 = [85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85]
; X32-SSE2-NEXT:    pand %xmm2, %xmm1
; X32-SSE2-NEXT:    psubb %xmm1, %xmm0
; X32-SSE2-NEXT:    movdqa {{.*#+}} xmm1 = [51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51]
; X32-SSE2-NEXT:    movdqa %xmm0, %xmm3
; X32-SSE2-NEXT:    pand %xmm1, %xmm3
; X32-SSE2-NEXT:    psrlw $2, %xmm0
; X32-SSE2-NEXT:    pand %xmm1, %xmm0
; X32-SSE2-NEXT:    paddb %xmm3, %xmm0
; X32-SSE2-NEXT:    movdqa %xmm0, %xmm3
; X32-SSE2-NEXT:    psrlw $4, %xmm3
; X32-SSE2-NEXT:    paddb %xmm0, %xmm3
; X32-SSE2-NEXT:    movdqa {{.*#+}} xmm0 = [15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15]
; X32-SSE2-NEXT:    pand %xmm0, %xmm3
; X32-SSE2-NEXT:    pxor %xmm4, %xmm4
; X32-SSE2-NEXT:    psadbw %xmm4, %xmm3
; X32-SSE2-NEXT:    movd %xmm3, %ecx
; X32-SSE2-NEXT:    movq {{.*#+}} xmm3 = mem[0],zero
; X32-SSE2-NEXT:    movdqa %xmm3, %xmm5
; X32-SSE2-NEXT:    psrlw $1, %xmm5
; X32-SSE2-NEXT:    pand %xmm2, %xmm5
; X32-SSE2-NEXT:    psubb %xmm5, %xmm3
; X32-SSE2-NEXT:    movdqa %xmm3, %xmm2
; X32-SSE2-NEXT:    pand %xmm1, %xmm2
; X32-SSE2-NEXT:    psrlw $2, %xmm3
; X32-SSE2-NEXT:    pand %xmm1, %xmm3
; X32-SSE2-NEXT:    paddb %xmm2, %xmm3
; X32-SSE2-NEXT:    movdqa %xmm3, %xmm1
; X32-SSE2-NEXT:    psrlw $4, %xmm1
; X32-SSE2-NEXT:    paddb %xmm3, %xmm1
; X32-SSE2-NEXT:    pand %xmm0, %xmm1
; X32-SSE2-NEXT:    psadbw %xmm4, %xmm1
; X32-SSE2-NEXT:    movd %xmm1, %edx
; X32-SSE2-NEXT:    addl %ecx, %edx
; X32-SSE2-NEXT:    xorl %ecx, %ecx
; X32-SSE2-NEXT:    movl %ecx, 12(%eax)
; X32-SSE2-NEXT:    movl %ecx, 8(%eax)
; X32-SSE2-NEXT:    movl %ecx, 4(%eax)
; X32-SSE2-NEXT:    movl %edx, (%eax)
; X32-SSE2-NEXT:    retl $4
;
; X32-SSSE3-LABEL: cnt128_optsize:
; X32-SSSE3:       # %bb.0:
; X32-SSSE3-NEXT:    movl {{[0-9]+}}(%esp), %eax
; X32-SSSE3-NEXT:    movdqa {{.*#+}} xmm0 = [15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15]
; X32-SSSE3-NEXT:    movq {{.*#+}} xmm1 = mem[0],zero
; X32-SSSE3-NEXT:    movdqa %xmm1, %xmm2
; X32-SSSE3-NEXT:    pand %xmm0, %xmm2
; X32-SSSE3-NEXT:    movdqa {{.*#+}} xmm3 = [0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4]
; X32-SSSE3-NEXT:    movdqa %xmm3, %xmm4
; X32-SSSE3-NEXT:    pshufb %xmm2, %xmm4
; X32-SSSE3-NEXT:    psrlw $4, %xmm1
; X32-SSSE3-NEXT:    pand %xmm0, %xmm1
; X32-SSSE3-NEXT:    movdqa %xmm3, %xmm2
; X32-SSSE3-NEXT:    pshufb %xmm1, %xmm2
; X32-SSSE3-NEXT:    paddb %xmm4, %xmm2
; X32-SSSE3-NEXT:    pxor %xmm1, %xmm1
; X32-SSSE3-NEXT:    psadbw %xmm1, %xmm2
; X32-SSSE3-NEXT:    movd %xmm2, %ecx
; X32-SSSE3-NEXT:    movq {{.*#+}} xmm2 = mem[0],zero
; X32-SSSE3-NEXT:    movdqa %xmm2, %xmm4
; X32-SSSE3-NEXT:    pand %xmm0, %xmm4
; X32-SSSE3-NEXT:    movdqa %xmm3, %xmm5
; X32-SSSE3-NEXT:    pshufb %xmm4, %xmm5
; X32-SSSE3-NEXT:    psrlw $4, %xmm2
; X32-SSSE3-NEXT:    pand %xmm0, %xmm2
; X32-SSSE3-NEXT:    pshufb %xmm2, %xmm3
; X32-SSSE3-NEXT:    paddb %xmm5, %xmm3
; X32-SSSE3-NEXT:    psadbw %xmm1, %xmm3
; X32-SSSE3-NEXT:    movd %xmm3, %edx
; X32-SSSE3-NEXT:    addl %ecx, %edx
; X32-SSSE3-NEXT:    xorl %ecx, %ecx
; X32-SSSE3-NEXT:    movl %ecx, 12(%eax)
; X32-SSSE3-NEXT:    movl %ecx, 8(%eax)
; X32-SSSE3-NEXT:    movl %ecx, 4(%eax)
; X32-SSSE3-NEXT:    movl %edx, (%eax)
; X32-SSSE3-NEXT:    retl $4
  %cnt = tail call i128 @llvm.ctpop.i128(i128 %x)
  ret i128 %cnt
}

declare i8 @llvm.ctpop.i8(i8) nounwind readnone
declare i16 @llvm.ctpop.i16(i16) nounwind readnone
declare i32 @llvm.ctpop.i32(i32) nounwind readnone
declare i64 @llvm.ctpop.i64(i64) nounwind readnone
declare i128 @llvm.ctpop.i128(i128) nounwind readnone
