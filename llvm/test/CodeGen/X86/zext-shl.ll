; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-unknown-unknown | FileCheck %s

define i32 @t1(i8 zeroext %x) nounwind {
; CHECK-LABEL: t1:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movzbl {{[0-9]+}}(%esp), %eax
; CHECK-NEXT:    shll $5, %eax
; CHECK-NEXT:    retl
  %t0 = zext i8 %x to i16
  %t1 = shl i16 %t0, 5
  %t2 = zext i16 %t1 to i32
  ret i32 %t2
}

define i32 @t2(i8 zeroext %x) nounwind {
; CHECK-LABEL: t2:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movzbl {{[0-9]+}}(%esp), %eax
; CHECK-NEXT:    shrl $3, %eax
; CHECK-NEXT:    retl
  %t0 = zext i8 %x to i16
  %t1 = lshr i16 %t0, 3
  %t2 = zext i16 %t1 to i32
  ret i32 %t2
}
