; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.Vector2 = type { float, float }

@.str = private unnamed_addr constant [11 x i8] c"Flood Fill\00", align 1
@.str.1 = private unnamed_addr constant [3 x i8] c"%f\00", align 1
@.str.2 = private unnamed_addr constant [13 x i8] c"Hello World\0A\00", align 1

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i32 @test(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = load i32, ptr %2, align 4
  %4 = load i32, ptr %2, align 4
  %5 = mul nsw i32 %3, %4
  %6 = sdiv i32 %5, 2
  %7 = add nsw i32 %6, 1
  ret i32 %7
}

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i32 @main() #1 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca %struct.Vector2, align 4
  store i32 0, ptr %1, align 4
  store i32 2, ptr %2, align 4
  call void @InitWindow(i32 noundef 800, i32 noundef 600, ptr noundef @.str)
  %4 = call <2 x float> @GetMousePosition()
  store <2 x float> %4, ptr %3, align 4
  %5 = getelementptr inbounds nuw %struct.Vector2, ptr %3, i32 0, i32 0
  %6 = load float, ptr %5, align 4
  %7 = fpext float %6 to double
  %8 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, double noundef %7)
  %9 = call i32 @rand() #4
  %10 = icmp eq i32 %9, 2
  br i1 %10, label %11, label %13

11:                                               ; preds = %0
  %12 = call i32 @test(i32 noundef 2)
  br label %15

13:                                               ; preds = %0
  %14 = call i32 @test(i32 noundef 5)
  br label %15

15:                                               ; preds = %13, %11
  %16 = call i32 (ptr, ...) @printf(ptr noundef @.str.2)
  ret i32 0
}

declare void @InitWindow(i32 noundef, i32 noundef, ptr noundef) #2

declare <2 x float> @GetMousePosition() #2

declare i32 @printf(ptr noundef, ...) #2

; Function Attrs: nounwind
declare i32 @rand() #3

attributes #0 = { noinline nounwind optnone sspstrong uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { noinline nounwind optnone sspstrong uwtable "frame-pointer"="all" "min-legal-vector-width"="64" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 22.1.1"}
