; ModuleID = 'OpenCL_14657785198273740405_simple'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%0 = type { i8*, i8*, i8*, i8*, i32 }

@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
; @llvm.global.annotations = appending global [1 x %0] [%0 { i8* bitcast (void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*)* @__OpenCL_simple_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"

; Function Attrs: nounwind
define void @__OpenCL_simple_kernel(i32 addrspace(1)* nocapture %A, i32 addrspace(1)* nocapture %B, i32 addrspace(1)* nocapture %C) #0 {
entry:
  %call1 = tail call i32 @get_local_id_simddim(i32 0) #0
  %0 = sext i32 %call1 to i64
  %arrayidx = getelementptr i32 addrspace(1)* %A, i64 %0
  %arrayidx4 = getelementptr i32 addrspace(1)* %B, i64 %0
  %tmp5 = load i32 addrspace(1)* %arrayidx4, align 4
  %arrayidx8 = getelementptr i32 addrspace(1)* %C, i64 %0
  %tmp9 = load i32 addrspace(1)* %arrayidx8, align 4
  %tmp10 = add nsw i32 %tmp9, %tmp5
  store i32 %tmp10, i32 addrspace(1)* %arrayidx, align 4
  ret void
}

; define void @simple_SIMD(i32 addrspace(1)* nocapture %A, i32 addrspace(1)* nocapture %B, i32 addrspace(1)* nocapture %C) {
; entry.:
;   %call1. = tail call i32 @get_local_id_simddim(i32 0)
;   %0 = sext i32 %call1. to i64
;   %1 = getelementptr i32 addrspace(1)* %A, i64 %0
;   %pktPtrCast = bitcast i32 addrspace(1)* %1 to <8 x i32> addrspace(1)*
;   %2 = getelementptr i32 addrspace(1)* %B, i64 %0
;   %pktPtrCast1 = bitcast i32 addrspace(1)* %2 to <8 x i32> addrspace(1)*
;   %tmp5. = load <8 x i32> addrspace(1)* %pktPtrCast1, align 32
;   %3 = getelementptr i32 addrspace(1)* %C, i64 %0
;   %pktPtrCast2 = bitcast i32 addrspace(1)* %3 to <8 x i32> addrspace(1)*
;   %tmp9. = load <8 x i32> addrspace(1)* %pktPtrCast2, align 32
;   %tmp10. = add nsw <8 x i32> %tmp9., %tmp5.
;   store <8 x i32> %tmp10., <8 x i32> addrspace(1)* %pktPtrCast, align 32
;   ret void
; }

declare i32 @get_local_id_simddim(i32)

; Function Attrs: nounwind
define void @simple_wrapper({ i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)* }* noalias nocapture %arg_struct, i32 %get_work_dim, i32* nocapture %get_global_size, i32* nocapture %get_local_size, i32* nocapture %get_group_id) #0 {
entry:
  %0 = getelementptr { i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)* }* %arg_struct, i64 0, i32 0
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr { i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)* }* %arg_struct, i64 0, i32 1
  %3 = load i32 addrspace(1)** %2, align 8
  %4 = getelementptr { i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)* }* %arg_struct, i64 0, i32 2
  %5 = load i32 addrspace(1)** %4, align 8
  %local_size_0 = load i32* %get_local_size, align 16
  br label %entry.header.loop

entry.header.loop:                                ; preds = %entry, %entry.header.loop
  %local_id_0 = phi i32 [ 0, %entry ], [ %inc, %entry.header.loop ]
  %6 = sext i32 %local_id_0 to i64
  %7 = getelementptr i32 addrspace(1)* %1, i64 %6
  %pktPtrCast.i = bitcast i32 addrspace(1)* %7 to <8 x i32> addrspace(1)*
  %8 = getelementptr i32 addrspace(1)* %3, i64 %6
  %pktPtrCast1.i = bitcast i32 addrspace(1)* %8 to <8 x i32> addrspace(1)*
  %tmp5..i = load <8 x i32> addrspace(1)* %pktPtrCast1.i, align 32
  %9 = getelementptr i32 addrspace(1)* %5, i64 %6
  %pktPtrCast2.i = bitcast i32 addrspace(1)* %9 to <8 x i32> addrspace(1)*
  %tmp9..i = load <8 x i32> addrspace(1)* %pktPtrCast2.i, align 32
  %tmp10..i = add nsw <8 x i32> %tmp9..i, %tmp5..i
  store <8 x i32> %tmp10..i, <8 x i32> addrspace(1)* %pktPtrCast.i, align 32
  %inc = add i32 %local_id_0, 8
  %exitcond = icmp ult i32 %inc, %local_size_0
  br i1 %exitcond, label %entry.header.loop, label %exit

exit:                                             ; preds = %entry.header.loop
  ret void
}

attributes #0 = { nounwind }
