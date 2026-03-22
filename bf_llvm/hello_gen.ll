target triple = "x86_64-pc-linux-gnu"
define i32 @main() {
    %ptr = alloca i8*, align 8
    %data_ptr = call i8* @calloc(i64 106, i64 1)
    store i8* %data_ptr, i8** %ptr, align 8
    %1 = load i8*, i8** %ptr, align 8
    %2 = load i8, i8* %1, align 1
    %3 = add i8 %2, 1
    store i8 %3, i8* %1, align 1
    %4 = load i8*, i8** %ptr, align 8
    %5 = load i8, i8* %4, align 1
    %6 = add i8 %5, 1
    store i8 %6, i8* %4, align 1
    %7 = load i8*, i8** %ptr, align 8
    %8 = load i8, i8* %7, align 1
    %9 = add i8 %8, 1
    store i8 %9, i8* %7, align 1
    %10 = load i8*, i8** %ptr, align 8
    %11 = load i8, i8* %10, align 1
    %12 = add i8 %11, 1
    store i8 %12, i8* %10, align 1
    %13 = load i8*, i8** %ptr, align 8
    %14 = load i8, i8* %13, align 1
    %15 = add i8 %14, 1
    store i8 %15, i8* %13, align 1
    %16 = load i8*, i8** %ptr, align 8
    %17 = load i8, i8* %16, align 1
    %18 = add i8 %17, 1
    store i8 %18, i8* %16, align 1
    %19 = load i8*, i8** %ptr, align 8
    %20 = load i8, i8* %19, align 1
    %21 = add i8 %20, 1
    store i8 %21, i8* %19, align 1
    %22 = load i8*, i8** %ptr, align 8
    %23 = load i8, i8* %22, align 1
    %24 = add i8 %23, 1
    store i8 %24, i8* %22, align 1
    br label %while_cond0
while_cond0: 
    %25 = load i8*, i8** %ptr, align 8
    %26 = load i8, i8* %25, align 1
    %27 = icmp ne i8 %26, 0
    br i1 %27, label %while_body0, label %while_end0
while_body0:
    %28 = load i8*, i8** %ptr, align 8
    %29 = getelementptr inbounds i8, i8* %28, i32 1
    store i8* %29, i8** %ptr, align 8
    %30 = load i8*, i8** %ptr, align 8
    %31 = load i8, i8* %30, align 1
    %32 = add i8 %31, 1
    store i8 %32, i8* %30, align 1
    %33 = load i8*, i8** %ptr, align 8
    %34 = load i8, i8* %33, align 1
    %35 = add i8 %34, 1
    store i8 %35, i8* %33, align 1
    %36 = load i8*, i8** %ptr, align 8
    %37 = load i8, i8* %36, align 1
    %38 = add i8 %37, 1
    store i8 %38, i8* %36, align 1
    %39 = load i8*, i8** %ptr, align 8
    %40 = load i8, i8* %39, align 1
    %41 = add i8 %40, 1
    store i8 %41, i8* %39, align 1
    br label %while_cond1
while_cond1: 
    %42 = load i8*, i8** %ptr, align 8
    %43 = load i8, i8* %42, align 1
    %44 = icmp ne i8 %43, 0
    br i1 %44, label %while_body1, label %while_end1
while_body1:
    %45 = load i8*, i8** %ptr, align 8
    %46 = getelementptr inbounds i8, i8* %45, i32 1
    store i8* %46, i8** %ptr, align 8
    %47 = load i8*, i8** %ptr, align 8
    %48 = load i8, i8* %47, align 1
    %49 = add i8 %48, 1
    store i8 %49, i8* %47, align 1
    %50 = load i8*, i8** %ptr, align 8
    %51 = load i8, i8* %50, align 1
    %52 = add i8 %51, 1
    store i8 %52, i8* %50, align 1
    %53 = load i8*, i8** %ptr, align 8
    %54 = getelementptr inbounds i8, i8* %53, i32 1
    store i8* %54, i8** %ptr, align 8
    %55 = load i8*, i8** %ptr, align 8
    %56 = load i8, i8* %55, align 1
    %57 = add i8 %56, 1
    store i8 %57, i8* %55, align 1
    %58 = load i8*, i8** %ptr, align 8
    %59 = load i8, i8* %58, align 1
    %60 = add i8 %59, 1
    store i8 %60, i8* %58, align 1
    %61 = load i8*, i8** %ptr, align 8
    %62 = load i8, i8* %61, align 1
    %63 = add i8 %62, 1
    store i8 %63, i8* %61, align 1
    %64 = load i8*, i8** %ptr, align 8
    %65 = getelementptr inbounds i8, i8* %64, i32 1
    store i8* %65, i8** %ptr, align 8
    %66 = load i8*, i8** %ptr, align 8
    %67 = load i8, i8* %66, align 1
    %68 = add i8 %67, 1
    store i8 %68, i8* %66, align 1
    %69 = load i8*, i8** %ptr, align 8
    %70 = load i8, i8* %69, align 1
    %71 = add i8 %70, 1
    store i8 %71, i8* %69, align 1
    %72 = load i8*, i8** %ptr, align 8
    %73 = load i8, i8* %72, align 1
    %74 = add i8 %73, 1
    store i8 %74, i8* %72, align 1
    %75 = load i8*, i8** %ptr, align 8
    %76 = getelementptr inbounds i8, i8* %75, i32 1
    store i8* %76, i8** %ptr, align 8
    %77 = load i8*, i8** %ptr, align 8
    %78 = load i8, i8* %77, align 1
    %79 = add i8 %78, 1
    store i8 %79, i8* %77, align 1
    %80 = load i8*, i8** %ptr, align 8
    %81 = getelementptr inbounds i8, i8* %80, i32 -1
    store i8* %81, i8** %ptr, align 8
    %82 = load i8*, i8** %ptr, align 8
    %83 = getelementptr inbounds i8, i8* %82, i32 -1
    store i8* %83, i8** %ptr, align 8
    %84 = load i8*, i8** %ptr, align 8
    %85 = getelementptr inbounds i8, i8* %84, i32 -1
    store i8* %85, i8** %ptr, align 8
    %86 = load i8*, i8** %ptr, align 8
    %87 = getelementptr inbounds i8, i8* %86, i32 -1
    store i8* %87, i8** %ptr, align 8
    %88 = load i8*, i8** %ptr, align 8
    %89 = load i8, i8* %88, align 1
    %90 = add i8 %89, -1
    store i8 %90, i8* %88, align 1
    br label %while_cond1
while_end1:
    %91 = load i8*, i8** %ptr, align 8
    %92 = getelementptr inbounds i8, i8* %91, i32 1
    store i8* %92, i8** %ptr, align 8
    %93 = load i8*, i8** %ptr, align 8
    %94 = load i8, i8* %93, align 1
    %95 = add i8 %94, 1
    store i8 %95, i8* %93, align 1
    %96 = load i8*, i8** %ptr, align 8
    %97 = getelementptr inbounds i8, i8* %96, i32 1
    store i8* %97, i8** %ptr, align 8
    %98 = load i8*, i8** %ptr, align 8
    %99 = load i8, i8* %98, align 1
    %100 = add i8 %99, 1
    store i8 %100, i8* %98, align 1
    %101 = load i8*, i8** %ptr, align 8
    %102 = getelementptr inbounds i8, i8* %101, i32 1
    store i8* %102, i8** %ptr, align 8
    %103 = load i8*, i8** %ptr, align 8
    %104 = load i8, i8* %103, align 1
    %105 = add i8 %104, -1
    store i8 %105, i8* %103, align 1
    %106 = load i8*, i8** %ptr, align 8
    %107 = getelementptr inbounds i8, i8* %106, i32 1
    store i8* %107, i8** %ptr, align 8
    %108 = load i8*, i8** %ptr, align 8
    %109 = getelementptr inbounds i8, i8* %108, i32 1
    store i8* %109, i8** %ptr, align 8
    %110 = load i8*, i8** %ptr, align 8
    %111 = load i8, i8* %110, align 1
    %112 = add i8 %111, 1
    store i8 %112, i8* %110, align 1
    br label %while_cond2
while_cond2: 
    %113 = load i8*, i8** %ptr, align 8
    %114 = load i8, i8* %113, align 1
    %115 = icmp ne i8 %114, 0
    br i1 %115, label %while_body2, label %while_end2
while_body2:
    %116 = load i8*, i8** %ptr, align 8
    %117 = getelementptr inbounds i8, i8* %116, i32 -1
    store i8* %117, i8** %ptr, align 8
    br label %while_cond2
while_end2:
    %118 = load i8*, i8** %ptr, align 8
    %119 = getelementptr inbounds i8, i8* %118, i32 -1
    store i8* %119, i8** %ptr, align 8
    %120 = load i8*, i8** %ptr, align 8
    %121 = load i8, i8* %120, align 1
    %122 = add i8 %121, -1
    store i8 %122, i8* %120, align 1
    br label %while_cond0
while_end0:
    %123 = load i8*, i8** %ptr, align 8
    %124 = getelementptr inbounds i8, i8* %123, i32 1
    store i8* %124, i8** %ptr, align 8
    %125 = load i8*, i8** %ptr, align 8
    %126 = getelementptr inbounds i8, i8* %125, i32 1
    store i8* %126, i8** %ptr, align 8
    %127 = load i8*, i8** %ptr, align 8
    %128 = load i8, i8* %127, align 1
    %129 = sext i8 %128 to i32
    %130 = call i32 @putchar(i32 %129)
    %131 = load i8*, i8** %ptr, align 8
    %132 = getelementptr inbounds i8, i8* %131, i32 1
    store i8* %132, i8** %ptr, align 8
    %133 = load i8*, i8** %ptr, align 8
    %134 = load i8, i8* %133, align 1
    %135 = add i8 %134, -1
    store i8 %135, i8* %133, align 1
    %136 = load i8*, i8** %ptr, align 8
    %137 = load i8, i8* %136, align 1
    %138 = add i8 %137, -1
    store i8 %138, i8* %136, align 1
    %139 = load i8*, i8** %ptr, align 8
    %140 = load i8, i8* %139, align 1
    %141 = add i8 %140, -1
    store i8 %141, i8* %139, align 1
    %142 = load i8*, i8** %ptr, align 8
    %143 = load i8, i8* %142, align 1
    %144 = sext i8 %143 to i32
    %145 = call i32 @putchar(i32 %144)
    %146 = load i8*, i8** %ptr, align 8
    %147 = load i8, i8* %146, align 1
    %148 = add i8 %147, 1
    store i8 %148, i8* %146, align 1
    %149 = load i8*, i8** %ptr, align 8
    %150 = load i8, i8* %149, align 1
    %151 = add i8 %150, 1
    store i8 %151, i8* %149, align 1
    %152 = load i8*, i8** %ptr, align 8
    %153 = load i8, i8* %152, align 1
    %154 = add i8 %153, 1
    store i8 %154, i8* %152, align 1
    %155 = load i8*, i8** %ptr, align 8
    %156 = load i8, i8* %155, align 1
    %157 = add i8 %156, 1
    store i8 %157, i8* %155, align 1
    %158 = load i8*, i8** %ptr, align 8
    %159 = load i8, i8* %158, align 1
    %160 = add i8 %159, 1
    store i8 %160, i8* %158, align 1
    %161 = load i8*, i8** %ptr, align 8
    %162 = load i8, i8* %161, align 1
    %163 = add i8 %162, 1
    store i8 %163, i8* %161, align 1
    %164 = load i8*, i8** %ptr, align 8
    %165 = load i8, i8* %164, align 1
    %166 = add i8 %165, 1
    store i8 %166, i8* %164, align 1
    %167 = load i8*, i8** %ptr, align 8
    %168 = load i8, i8* %167, align 1
    %169 = sext i8 %168 to i32
    %170 = call i32 @putchar(i32 %169)
    %171 = load i8*, i8** %ptr, align 8
    %172 = load i8, i8* %171, align 1
    %173 = sext i8 %172 to i32
    %174 = call i32 @putchar(i32 %173)
    %175 = load i8*, i8** %ptr, align 8
    %176 = load i8, i8* %175, align 1
    %177 = add i8 %176, 1
    store i8 %177, i8* %175, align 1
    %178 = load i8*, i8** %ptr, align 8
    %179 = load i8, i8* %178, align 1
    %180 = add i8 %179, 1
    store i8 %180, i8* %178, align 1
    %181 = load i8*, i8** %ptr, align 8
    %182 = load i8, i8* %181, align 1
    %183 = add i8 %182, 1
    store i8 %183, i8* %181, align 1
    %184 = load i8*, i8** %ptr, align 8
    %185 = load i8, i8* %184, align 1
    %186 = sext i8 %185 to i32
    %187 = call i32 @putchar(i32 %186)
    %188 = load i8*, i8** %ptr, align 8
    %189 = getelementptr inbounds i8, i8* %188, i32 1
    store i8* %189, i8** %ptr, align 8
    %190 = load i8*, i8** %ptr, align 8
    %191 = getelementptr inbounds i8, i8* %190, i32 1
    store i8* %191, i8** %ptr, align 8
    %192 = load i8*, i8** %ptr, align 8
    %193 = load i8, i8* %192, align 1
    %194 = sext i8 %193 to i32
    %195 = call i32 @putchar(i32 %194)
    %196 = load i8*, i8** %ptr, align 8
    %197 = getelementptr inbounds i8, i8* %196, i32 -1
    store i8* %197, i8** %ptr, align 8
    %198 = load i8*, i8** %ptr, align 8
    %199 = load i8, i8* %198, align 1
    %200 = add i8 %199, -1
    store i8 %200, i8* %198, align 1
    %201 = load i8*, i8** %ptr, align 8
    %202 = load i8, i8* %201, align 1
    %203 = sext i8 %202 to i32
    %204 = call i32 @putchar(i32 %203)
    %205 = load i8*, i8** %ptr, align 8
    %206 = getelementptr inbounds i8, i8* %205, i32 -1
    store i8* %206, i8** %ptr, align 8
    %207 = load i8*, i8** %ptr, align 8
    %208 = load i8, i8* %207, align 1
    %209 = sext i8 %208 to i32
    %210 = call i32 @putchar(i32 %209)
    %211 = load i8*, i8** %ptr, align 8
    %212 = load i8, i8* %211, align 1
    %213 = add i8 %212, 1
    store i8 %213, i8* %211, align 1
    %214 = load i8*, i8** %ptr, align 8
    %215 = load i8, i8* %214, align 1
    %216 = add i8 %215, 1
    store i8 %216, i8* %214, align 1
    %217 = load i8*, i8** %ptr, align 8
    %218 = load i8, i8* %217, align 1
    %219 = add i8 %218, 1
    store i8 %219, i8* %217, align 1
    %220 = load i8*, i8** %ptr, align 8
    %221 = load i8, i8* %220, align 1
    %222 = sext i8 %221 to i32
    %223 = call i32 @putchar(i32 %222)
    %224 = load i8*, i8** %ptr, align 8
    %225 = load i8, i8* %224, align 1
    %226 = add i8 %225, -1
    store i8 %226, i8* %224, align 1
    %227 = load i8*, i8** %ptr, align 8
    %228 = load i8, i8* %227, align 1
    %229 = add i8 %228, -1
    store i8 %229, i8* %227, align 1
    %230 = load i8*, i8** %ptr, align 8
    %231 = load i8, i8* %230, align 1
    %232 = add i8 %231, -1
    store i8 %232, i8* %230, align 1
    %233 = load i8*, i8** %ptr, align 8
    %234 = load i8, i8* %233, align 1
    %235 = add i8 %234, -1
    store i8 %235, i8* %233, align 1
    %236 = load i8*, i8** %ptr, align 8
    %237 = load i8, i8* %236, align 1
    %238 = add i8 %237, -1
    store i8 %238, i8* %236, align 1
    %239 = load i8*, i8** %ptr, align 8
    %240 = load i8, i8* %239, align 1
    %241 = add i8 %240, -1
    store i8 %241, i8* %239, align 1
    %242 = load i8*, i8** %ptr, align 8
    %243 = load i8, i8* %242, align 1
    %244 = sext i8 %243 to i32
    %245 = call i32 @putchar(i32 %244)
    %246 = load i8*, i8** %ptr, align 8
    %247 = load i8, i8* %246, align 1
    %248 = add i8 %247, -1
    store i8 %248, i8* %246, align 1
    %249 = load i8*, i8** %ptr, align 8
    %250 = load i8, i8* %249, align 1
    %251 = add i8 %250, -1
    store i8 %251, i8* %249, align 1
    %252 = load i8*, i8** %ptr, align 8
    %253 = load i8, i8* %252, align 1
    %254 = add i8 %253, -1
    store i8 %254, i8* %252, align 1
    %255 = load i8*, i8** %ptr, align 8
    %256 = load i8, i8* %255, align 1
    %257 = add i8 %256, -1
    store i8 %257, i8* %255, align 1
    %258 = load i8*, i8** %ptr, align 8
    %259 = load i8, i8* %258, align 1
    %260 = add i8 %259, -1
    store i8 %260, i8* %258, align 1
    %261 = load i8*, i8** %ptr, align 8
    %262 = load i8, i8* %261, align 1
    %263 = add i8 %262, -1
    store i8 %263, i8* %261, align 1
    %264 = load i8*, i8** %ptr, align 8
    %265 = load i8, i8* %264, align 1
    %266 = add i8 %265, -1
    store i8 %266, i8* %264, align 1
    %267 = load i8*, i8** %ptr, align 8
    %268 = load i8, i8* %267, align 1
    %269 = add i8 %268, -1
    store i8 %269, i8* %267, align 1
    %270 = load i8*, i8** %ptr, align 8
    %271 = load i8, i8* %270, align 1
    %272 = sext i8 %271 to i32
    %273 = call i32 @putchar(i32 %272)
    %274 = load i8*, i8** %ptr, align 8
    %275 = getelementptr inbounds i8, i8* %274, i32 1
    store i8* %275, i8** %ptr, align 8
    %276 = load i8*, i8** %ptr, align 8
    %277 = getelementptr inbounds i8, i8* %276, i32 1
    store i8* %277, i8** %ptr, align 8
    %278 = load i8*, i8** %ptr, align 8
    %279 = load i8, i8* %278, align 1
    %280 = add i8 %279, 1
    store i8 %280, i8* %278, align 1
    %281 = load i8*, i8** %ptr, align 8
    %282 = load i8, i8* %281, align 1
    %283 = sext i8 %282 to i32
    %284 = call i32 @putchar(i32 %283)
    %285 = load i8*, i8** %ptr, align 8
    %286 = getelementptr inbounds i8, i8* %285, i32 1
    store i8* %286, i8** %ptr, align 8
    %287 = load i8*, i8** %ptr, align 8
    %288 = load i8, i8* %287, align 1
    %289 = add i8 %288, 1
    store i8 %289, i8* %287, align 1
    %290 = load i8*, i8** %ptr, align 8
    %291 = load i8, i8* %290, align 1
    %292 = add i8 %291, 1
    store i8 %292, i8* %290, align 1
    %293 = load i8*, i8** %ptr, align 8
    %294 = load i8, i8* %293, align 1
    %295 = sext i8 %294 to i32
    %296 = call i32 @putchar(i32 %295)
 call void @free(i8* %data_ptr)
 ret i32 0
}
declare i8* @calloc(i64, i64)
declare void @free(i8*)
declare i32 @putchar(i32)
declare i32 @getchar()
