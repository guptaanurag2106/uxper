target triple = "x86_64-pc-linux-gnu"

@hello = constant [13 x i8] c"Hello World\0A\00"
@title = constant [11 x i8] c"Flood Fill\00"

@width = constant i32 800
@height = constant i32 600
@gridS = constant i32 10

%struct.Vector2 = type { float, float }

define ptr @grid_ptr_at(ptr %grid, i32 %gc, i32 %x, i32 %y) {
  %row = mul nsw i32 %y, %gc
  %idx = add nsw i32 %row, %x
  %idx64 = zext i32 %idx to i64
  %ptr = getelementptr inbounds i32, ptr %grid, i64 %idx64
  ret ptr %ptr
}

define void @floodfill_helper(ptr %grid, i32 %gc, i32 %gr, i32 %gridX, i32 %gridY, i32 %colour_old) {
  %x_lt0 = icmp slt i32 %gridX, 0
  %x_gt  = icmp sge i32 %gridX, %gc
  %x_out  = or i1 %x_lt0, %x_gt
  %y_lt0 = icmp slt i32 %gridY, 0
  %y_gt  = icmp sge i32 %gridY, %gr
  %y_out  = or i1 %y_lt0, %y_gt
  %out_bounds = or i1 %x_out, %y_out
  br i1 %out_bounds, label %ret, label %continue

continue:
  %colourptr = call ptr @grid_ptr_at(ptr %grid, i32 %gc, i32 %gridX, i32 %gridY)
  %colour = load i32, ptr %colourptr
  %uneq = icmp ne i32 %colour, %colour_old
  br i1 %uneq, label %ret, label %continue2

continue2:
  store i32 u0xFF00FFFF, ptr %colourptr
  %x1 = add i32 1, %gridX
  %xn1 = add i32 -1, %gridX
  %y1 = add i32 1, %gridY
  %yn1 = add i32 -1, %gridY

  call void @floodfill_helper(ptr %grid, i32 %gc, i32 %gr, i32 %x1, i32 %gridY, i32 %colour_old);
  call void @floodfill_helper(ptr %grid, i32 %gc, i32 %gr, i32 %xn1, i32 %gridY, i32 %colour_old);
  call void @floodfill_helper(ptr %grid, i32 %gc, i32 %gr, i32 %gridX, i32 %y1, i32 %colour_old);
  call void @floodfill_helper(ptr %grid, i32 %gc, i32 %gr, i32 %gridX, i32 %yn1, i32 %colour_old);
  br label %ret

ret:
  ret void
}

define void @floodfill(ptr %grid, i32 %gc, i32 %gr, i32 %gridX, i32 %gridY) {
  %colourptr = call ptr @grid_ptr_at(ptr %grid, i32 %gc, i32 %gridX, i32 %gridY)
  %colour = load i32, ptr %colourptr

  call void @floodfill_helper(ptr %grid, i32 %gc, i32 %gr, i32 %gridX, i32 %gridY, i32 %colour);
  ret void
}

define i32 @main() {
  call i32 @printf(ptr @hello)

  %vw = load i32, ptr @width
  %vh = load i32, ptr @height
  %vs = load i32, ptr @gridS
  %gc = sdiv i32 %vw, %vs
  %gr = sdiv i32 %vh, %vs

  %gridC = mul i32 %gc, %gr
  %bytes = mul i32 %gridC, 4
  %bytes64 = zext i32 %bytes to i64

  %grid = alloca i32, i32 %gridC, align 4
  call void @llvm.memset.p0.i64(ptr align 4 %grid, i8 0, i64 %bytes64, i1 false)

  call void @InitWindow(i32 %vw, i32 %vh, ptr @title)
  call void @SetTargetFPS(i32 60)

  %i = alloca i32;
  %j = alloca i32;
  br label %loop
loop:
  %4 = call i1 @WindowShouldClose()
  br i1 %4, label %end, label %inner

inner:
  call void @BeginDrawing()
  call void @ClearBackground(i32 u0xFF282828)

  store i32 0, ptr %i
  store i32 0, ptr %j
  br label %rows
  
rows:
  %vi = load i32, i32* %i
  %c1 = icmp slt i32 %vi, %gr
  store i32 0, ptr %j
  br i1 %c1, label %columns, label %continue
columns:
  %vj = load i32, i32* %j
  ;; draw with vi, vj
  %colour = call ptr @grid_ptr_at(ptr %grid, i32 %gc, i32 %vj, i32 %vi)
  %vcolour = load i32, ptr %colour
  ; %vcolour = add i32 0, u0xFF00FF00
  %x = mul nsw i32 %vj , %vs
  %y = mul nsw i32 %vi , %vs
  call void @DrawRectangle(i32 %x, i32 %y, i32 %vs, i32 %vs, i32 %vcolour)

  ;; j+=1
  %vji = add i32 %vj, 1
  store i32 %vji, i32* %j
  %c2 = icmp slt i32 %vji, %gc
  br i1 %c2, label %columns, label %rows_end
rows_end:
  ;; i+=1
  %vii = add i32 %vi, 1
  store i32 %vii, i32* %i
  br label %rows

continue:
;; mouse pressed
  %left = call i1 @IsMouseButtonDown(i32 0)
  %right = call i1 @IsMouseButtonReleased(i32 1)

  %mousePosV = call <2 x float> @GetMousePosition()
  %mx_f = extractelement <2 x float> %mousePosV, i32 0
  %my_f = extractelement <2 x float> %mousePosV, i32 1
  %mx = fptosi float %mx_f to i32
  %my = fptosi float %my_f to i32
  %mouseX = sdiv i32 %mx, %vs
  %mouseY = sdiv i32 %my, %vs

  ;; bounds check
  %x_ge0 = icmp sge i32 %mouseX, 0
  %x_lt  = icmp slt i32 %mouseX, %gc
  %x_ok  = and i1 %x_ge0, %x_lt
  %y_ge0 = icmp sge i32 %mouseY, 0
  %y_lt  = icmp slt i32 %mouseY, %gr
  %y_ok  = and i1 %y_ge0, %y_lt
  %in_bounds = and i1 %x_ok, %y_ok
  %do_left   = and i1 %left, %in_bounds
  %do_right   = and i1 %right, %in_bounds
  br i1 %do_left, label %left_click, label %temp

temp:
  br i1 %do_right, label %right_click, label %loop_end

left_click:
  %colourptr = call ptr @grid_ptr_at(ptr %grid, i32 %gc, i32 %mouseX, i32 %mouseY)
  store i32 u0xFF00FF00, ptr %colourptr
  br label %loop_end

right_click:
  call void @floodfill(ptr %grid, i32 %gc, i32 %gr, i32 %mouseX, i32 %mouseY)
  br label %loop_end

loop_end:
  call void @EndDrawing()

  br label %loop

end:
  call void @CloseWindow()
  ret i32 0
}

declare i32 @printf(ptr, ...)

declare void @InitWindow(i32, i32, ptr)
declare i1 @WindowShouldClose()
declare void @BeginDrawing()
declare void @ClearBackground(i32)
declare void @SetTargetFPS(i32)
declare void @DrawRectangle(i32, i32, i32, i32, i32)
declare i1 @IsMouseButtonDown(i32)
declare i1 @IsMouseButtonReleased(i32)
declare <2 x float> @GetMousePosition()
declare void @EndDrawing()
declare void @CloseWindow()
