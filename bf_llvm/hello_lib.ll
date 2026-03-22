; ModuleID = 'bf_module'
source_filename = "bf_module"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

declare i32 @putchar(i32)

declare i32 @getchar()

define i32 @main() {
entry:
  %tape = alloca i8, i32 30000, align 1
  %idx = alloca i32, align 4
  store i32 0, ptr %idx, align 4
  call void @llvm.memset.p0.i64(ptr align 1 %tape, i8 0, i64 30000, i1 false)
  %idx1 = load i32, ptr %idx, align 4
  %cell.ptr = getelementptr inbounds i8, ptr %tape, i32 %idx1
  %cell = load i8, ptr %cell.ptr, align 1
  %cell.inc = add i8 %cell, 1
  store i8 %cell.inc, ptr %cell.ptr, align 1
  %idx2 = load i32, ptr %idx, align 4
  %cell.ptr3 = getelementptr inbounds i8, ptr %tape, i32 %idx2
  %cell4 = load i8, ptr %cell.ptr3, align 1
  %cell.inc5 = add i8 %cell4, 1
  store i8 %cell.inc5, ptr %cell.ptr3, align 1
  %idx6 = load i32, ptr %idx, align 4
  %cell.ptr7 = getelementptr inbounds i8, ptr %tape, i32 %idx6
  %cell8 = load i8, ptr %cell.ptr7, align 1
  %cell.inc9 = add i8 %cell8, 1
  store i8 %cell.inc9, ptr %cell.ptr7, align 1
  %idx10 = load i32, ptr %idx, align 4
  %cell.ptr11 = getelementptr inbounds i8, ptr %tape, i32 %idx10
  %cell12 = load i8, ptr %cell.ptr11, align 1
  %cell.inc13 = add i8 %cell12, 1
  store i8 %cell.inc13, ptr %cell.ptr11, align 1
  %idx14 = load i32, ptr %idx, align 4
  %cell.ptr15 = getelementptr inbounds i8, ptr %tape, i32 %idx14
  %cell16 = load i8, ptr %cell.ptr15, align 1
  %cell.inc17 = add i8 %cell16, 1
  store i8 %cell.inc17, ptr %cell.ptr15, align 1
  %idx18 = load i32, ptr %idx, align 4
  %cell.ptr19 = getelementptr inbounds i8, ptr %tape, i32 %idx18
  %cell20 = load i8, ptr %cell.ptr19, align 1
  %cell.inc21 = add i8 %cell20, 1
  store i8 %cell.inc21, ptr %cell.ptr19, align 1
  %idx22 = load i32, ptr %idx, align 4
  %cell.ptr23 = getelementptr inbounds i8, ptr %tape, i32 %idx22
  %cell24 = load i8, ptr %cell.ptr23, align 1
  %cell.inc25 = add i8 %cell24, 1
  store i8 %cell.inc25, ptr %cell.ptr23, align 1
  %idx26 = load i32, ptr %idx, align 4
  %cell.ptr27 = getelementptr inbounds i8, ptr %tape, i32 %idx26
  %cell28 = load i8, ptr %cell.ptr27, align 1
  %cell.inc29 = add i8 %cell28, 1
  store i8 %cell.inc29, ptr %cell.ptr27, align 1
  br label %loop.cond

loop.cond:                                        ; preds = %loop.end139, %entry
  %idx30 = load i32, ptr %idx, align 4
  %cell.ptr31 = getelementptr inbounds i8, ptr %tape, i32 %idx30
  %cell32 = load i8, ptr %cell.ptr31, align 1
  %cell.nz = icmp ne i8 %cell32, 0
  br i1 %cell.nz, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.cond
  %idx33 = load i32, ptr %idx, align 4
  %idx.inc = add i32 %idx33, 1
  store i32 %idx.inc, ptr %idx, align 4
  %idx34 = load i32, ptr %idx, align 4
  %cell.ptr35 = getelementptr inbounds i8, ptr %tape, i32 %idx34
  %cell36 = load i8, ptr %cell.ptr35, align 1
  %cell.inc37 = add i8 %cell36, 1
  store i8 %cell.inc37, ptr %cell.ptr35, align 1
  %idx38 = load i32, ptr %idx, align 4
  %cell.ptr39 = getelementptr inbounds i8, ptr %tape, i32 %idx38
  %cell40 = load i8, ptr %cell.ptr39, align 1
  %cell.inc41 = add i8 %cell40, 1
  store i8 %cell.inc41, ptr %cell.ptr39, align 1
  %idx42 = load i32, ptr %idx, align 4
  %cell.ptr43 = getelementptr inbounds i8, ptr %tape, i32 %idx42
  %cell44 = load i8, ptr %cell.ptr43, align 1
  %cell.inc45 = add i8 %cell44, 1
  store i8 %cell.inc45, ptr %cell.ptr43, align 1
  %idx46 = load i32, ptr %idx, align 4
  %cell.ptr47 = getelementptr inbounds i8, ptr %tape, i32 %idx46
  %cell48 = load i8, ptr %cell.ptr47, align 1
  %cell.inc49 = add i8 %cell48, 1
  store i8 %cell.inc49, ptr %cell.ptr47, align 1
  br label %loop.cond50

loop.end:                                         ; preds = %loop.cond
  %idx152 = load i32, ptr %idx, align 4
  %idx.inc153 = add i32 %idx152, 1
  store i32 %idx.inc153, ptr %idx, align 4
  %idx154 = load i32, ptr %idx, align 4
  %idx.inc155 = add i32 %idx154, 1
  store i32 %idx.inc155, ptr %idx, align 4
  %idx156 = load i32, ptr %idx, align 4
  %cell.ptr157 = getelementptr inbounds i8, ptr %tape, i32 %idx156
  %cell158 = load i8, ptr %cell.ptr157, align 1
  %cell.zext = zext i8 %cell158 to i32
  %0 = call i32 @putchar(i32 %cell.zext)
  %idx159 = load i32, ptr %idx, align 4
  %idx.inc160 = add i32 %idx159, 1
  store i32 %idx.inc160, ptr %idx, align 4
  %idx161 = load i32, ptr %idx, align 4
  %cell.ptr162 = getelementptr inbounds i8, ptr %tape, i32 %idx161
  %cell163 = load i8, ptr %cell.ptr162, align 1
  %cell.dec164 = sub i8 %cell163, 1
  store i8 %cell.dec164, ptr %cell.ptr162, align 1
  %idx165 = load i32, ptr %idx, align 4
  %cell.ptr166 = getelementptr inbounds i8, ptr %tape, i32 %idx165
  %cell167 = load i8, ptr %cell.ptr166, align 1
  %cell.dec168 = sub i8 %cell167, 1
  store i8 %cell.dec168, ptr %cell.ptr166, align 1
  %idx169 = load i32, ptr %idx, align 4
  %cell.ptr170 = getelementptr inbounds i8, ptr %tape, i32 %idx169
  %cell171 = load i8, ptr %cell.ptr170, align 1
  %cell.dec172 = sub i8 %cell171, 1
  store i8 %cell.dec172, ptr %cell.ptr170, align 1
  %idx173 = load i32, ptr %idx, align 4
  %cell.ptr174 = getelementptr inbounds i8, ptr %tape, i32 %idx173
  %cell175 = load i8, ptr %cell.ptr174, align 1
  %cell.zext176 = zext i8 %cell175 to i32
  %1 = call i32 @putchar(i32 %cell.zext176)
  %idx177 = load i32, ptr %idx, align 4
  %cell.ptr178 = getelementptr inbounds i8, ptr %tape, i32 %idx177
  %cell179 = load i8, ptr %cell.ptr178, align 1
  %cell.inc180 = add i8 %cell179, 1
  store i8 %cell.inc180, ptr %cell.ptr178, align 1
  %idx181 = load i32, ptr %idx, align 4
  %cell.ptr182 = getelementptr inbounds i8, ptr %tape, i32 %idx181
  %cell183 = load i8, ptr %cell.ptr182, align 1
  %cell.inc184 = add i8 %cell183, 1
  store i8 %cell.inc184, ptr %cell.ptr182, align 1
  %idx185 = load i32, ptr %idx, align 4
  %cell.ptr186 = getelementptr inbounds i8, ptr %tape, i32 %idx185
  %cell187 = load i8, ptr %cell.ptr186, align 1
  %cell.inc188 = add i8 %cell187, 1
  store i8 %cell.inc188, ptr %cell.ptr186, align 1
  %idx189 = load i32, ptr %idx, align 4
  %cell.ptr190 = getelementptr inbounds i8, ptr %tape, i32 %idx189
  %cell191 = load i8, ptr %cell.ptr190, align 1
  %cell.inc192 = add i8 %cell191, 1
  store i8 %cell.inc192, ptr %cell.ptr190, align 1
  %idx193 = load i32, ptr %idx, align 4
  %cell.ptr194 = getelementptr inbounds i8, ptr %tape, i32 %idx193
  %cell195 = load i8, ptr %cell.ptr194, align 1
  %cell.inc196 = add i8 %cell195, 1
  store i8 %cell.inc196, ptr %cell.ptr194, align 1
  %idx197 = load i32, ptr %idx, align 4
  %cell.ptr198 = getelementptr inbounds i8, ptr %tape, i32 %idx197
  %cell199 = load i8, ptr %cell.ptr198, align 1
  %cell.inc200 = add i8 %cell199, 1
  store i8 %cell.inc200, ptr %cell.ptr198, align 1
  %idx201 = load i32, ptr %idx, align 4
  %cell.ptr202 = getelementptr inbounds i8, ptr %tape, i32 %idx201
  %cell203 = load i8, ptr %cell.ptr202, align 1
  %cell.inc204 = add i8 %cell203, 1
  store i8 %cell.inc204, ptr %cell.ptr202, align 1
  %idx205 = load i32, ptr %idx, align 4
  %cell.ptr206 = getelementptr inbounds i8, ptr %tape, i32 %idx205
  %cell207 = load i8, ptr %cell.ptr206, align 1
  %cell.zext208 = zext i8 %cell207 to i32
  %2 = call i32 @putchar(i32 %cell.zext208)
  %idx209 = load i32, ptr %idx, align 4
  %cell.ptr210 = getelementptr inbounds i8, ptr %tape, i32 %idx209
  %cell211 = load i8, ptr %cell.ptr210, align 1
  %cell.zext212 = zext i8 %cell211 to i32
  %3 = call i32 @putchar(i32 %cell.zext212)
  %idx213 = load i32, ptr %idx, align 4
  %cell.ptr214 = getelementptr inbounds i8, ptr %tape, i32 %idx213
  %cell215 = load i8, ptr %cell.ptr214, align 1
  %cell.inc216 = add i8 %cell215, 1
  store i8 %cell.inc216, ptr %cell.ptr214, align 1
  %idx217 = load i32, ptr %idx, align 4
  %cell.ptr218 = getelementptr inbounds i8, ptr %tape, i32 %idx217
  %cell219 = load i8, ptr %cell.ptr218, align 1
  %cell.inc220 = add i8 %cell219, 1
  store i8 %cell.inc220, ptr %cell.ptr218, align 1
  %idx221 = load i32, ptr %idx, align 4
  %cell.ptr222 = getelementptr inbounds i8, ptr %tape, i32 %idx221
  %cell223 = load i8, ptr %cell.ptr222, align 1
  %cell.inc224 = add i8 %cell223, 1
  store i8 %cell.inc224, ptr %cell.ptr222, align 1
  %idx225 = load i32, ptr %idx, align 4
  %cell.ptr226 = getelementptr inbounds i8, ptr %tape, i32 %idx225
  %cell227 = load i8, ptr %cell.ptr226, align 1
  %cell.zext228 = zext i8 %cell227 to i32
  %4 = call i32 @putchar(i32 %cell.zext228)
  %idx229 = load i32, ptr %idx, align 4
  %idx.inc230 = add i32 %idx229, 1
  store i32 %idx.inc230, ptr %idx, align 4
  %idx231 = load i32, ptr %idx, align 4
  %idx.inc232 = add i32 %idx231, 1
  store i32 %idx.inc232, ptr %idx, align 4
  %idx233 = load i32, ptr %idx, align 4
  %cell.ptr234 = getelementptr inbounds i8, ptr %tape, i32 %idx233
  %cell235 = load i8, ptr %cell.ptr234, align 1
  %cell.zext236 = zext i8 %cell235 to i32
  %5 = call i32 @putchar(i32 %cell.zext236)
  %idx237 = load i32, ptr %idx, align 4
  %idx.dec238 = sub i32 %idx237, 1
  store i32 %idx.dec238, ptr %idx, align 4
  %idx239 = load i32, ptr %idx, align 4
  %cell.ptr240 = getelementptr inbounds i8, ptr %tape, i32 %idx239
  %cell241 = load i8, ptr %cell.ptr240, align 1
  %cell.dec242 = sub i8 %cell241, 1
  store i8 %cell.dec242, ptr %cell.ptr240, align 1
  %idx243 = load i32, ptr %idx, align 4
  %cell.ptr244 = getelementptr inbounds i8, ptr %tape, i32 %idx243
  %cell245 = load i8, ptr %cell.ptr244, align 1
  %cell.zext246 = zext i8 %cell245 to i32
  %6 = call i32 @putchar(i32 %cell.zext246)
  %idx247 = load i32, ptr %idx, align 4
  %idx.dec248 = sub i32 %idx247, 1
  store i32 %idx.dec248, ptr %idx, align 4
  %idx249 = load i32, ptr %idx, align 4
  %cell.ptr250 = getelementptr inbounds i8, ptr %tape, i32 %idx249
  %cell251 = load i8, ptr %cell.ptr250, align 1
  %cell.zext252 = zext i8 %cell251 to i32
  %7 = call i32 @putchar(i32 %cell.zext252)
  %idx253 = load i32, ptr %idx, align 4
  %cell.ptr254 = getelementptr inbounds i8, ptr %tape, i32 %idx253
  %cell255 = load i8, ptr %cell.ptr254, align 1
  %cell.inc256 = add i8 %cell255, 1
  store i8 %cell.inc256, ptr %cell.ptr254, align 1
  %idx257 = load i32, ptr %idx, align 4
  %cell.ptr258 = getelementptr inbounds i8, ptr %tape, i32 %idx257
  %cell259 = load i8, ptr %cell.ptr258, align 1
  %cell.inc260 = add i8 %cell259, 1
  store i8 %cell.inc260, ptr %cell.ptr258, align 1
  %idx261 = load i32, ptr %idx, align 4
  %cell.ptr262 = getelementptr inbounds i8, ptr %tape, i32 %idx261
  %cell263 = load i8, ptr %cell.ptr262, align 1
  %cell.inc264 = add i8 %cell263, 1
  store i8 %cell.inc264, ptr %cell.ptr262, align 1
  %idx265 = load i32, ptr %idx, align 4
  %cell.ptr266 = getelementptr inbounds i8, ptr %tape, i32 %idx265
  %cell267 = load i8, ptr %cell.ptr266, align 1
  %cell.zext268 = zext i8 %cell267 to i32
  %8 = call i32 @putchar(i32 %cell.zext268)
  %idx269 = load i32, ptr %idx, align 4
  %cell.ptr270 = getelementptr inbounds i8, ptr %tape, i32 %idx269
  %cell271 = load i8, ptr %cell.ptr270, align 1
  %cell.dec272 = sub i8 %cell271, 1
  store i8 %cell.dec272, ptr %cell.ptr270, align 1
  %idx273 = load i32, ptr %idx, align 4
  %cell.ptr274 = getelementptr inbounds i8, ptr %tape, i32 %idx273
  %cell275 = load i8, ptr %cell.ptr274, align 1
  %cell.dec276 = sub i8 %cell275, 1
  store i8 %cell.dec276, ptr %cell.ptr274, align 1
  %idx277 = load i32, ptr %idx, align 4
  %cell.ptr278 = getelementptr inbounds i8, ptr %tape, i32 %idx277
  %cell279 = load i8, ptr %cell.ptr278, align 1
  %cell.dec280 = sub i8 %cell279, 1
  store i8 %cell.dec280, ptr %cell.ptr278, align 1
  %idx281 = load i32, ptr %idx, align 4
  %cell.ptr282 = getelementptr inbounds i8, ptr %tape, i32 %idx281
  %cell283 = load i8, ptr %cell.ptr282, align 1
  %cell.dec284 = sub i8 %cell283, 1
  store i8 %cell.dec284, ptr %cell.ptr282, align 1
  %idx285 = load i32, ptr %idx, align 4
  %cell.ptr286 = getelementptr inbounds i8, ptr %tape, i32 %idx285
  %cell287 = load i8, ptr %cell.ptr286, align 1
  %cell.dec288 = sub i8 %cell287, 1
  store i8 %cell.dec288, ptr %cell.ptr286, align 1
  %idx289 = load i32, ptr %idx, align 4
  %cell.ptr290 = getelementptr inbounds i8, ptr %tape, i32 %idx289
  %cell291 = load i8, ptr %cell.ptr290, align 1
  %cell.dec292 = sub i8 %cell291, 1
  store i8 %cell.dec292, ptr %cell.ptr290, align 1
  %idx293 = load i32, ptr %idx, align 4
  %cell.ptr294 = getelementptr inbounds i8, ptr %tape, i32 %idx293
  %cell295 = load i8, ptr %cell.ptr294, align 1
  %cell.zext296 = zext i8 %cell295 to i32
  %9 = call i32 @putchar(i32 %cell.zext296)
  %idx297 = load i32, ptr %idx, align 4
  %cell.ptr298 = getelementptr inbounds i8, ptr %tape, i32 %idx297
  %cell299 = load i8, ptr %cell.ptr298, align 1
  %cell.dec300 = sub i8 %cell299, 1
  store i8 %cell.dec300, ptr %cell.ptr298, align 1
  %idx301 = load i32, ptr %idx, align 4
  %cell.ptr302 = getelementptr inbounds i8, ptr %tape, i32 %idx301
  %cell303 = load i8, ptr %cell.ptr302, align 1
  %cell.dec304 = sub i8 %cell303, 1
  store i8 %cell.dec304, ptr %cell.ptr302, align 1
  %idx305 = load i32, ptr %idx, align 4
  %cell.ptr306 = getelementptr inbounds i8, ptr %tape, i32 %idx305
  %cell307 = load i8, ptr %cell.ptr306, align 1
  %cell.dec308 = sub i8 %cell307, 1
  store i8 %cell.dec308, ptr %cell.ptr306, align 1
  %idx309 = load i32, ptr %idx, align 4
  %cell.ptr310 = getelementptr inbounds i8, ptr %tape, i32 %idx309
  %cell311 = load i8, ptr %cell.ptr310, align 1
  %cell.dec312 = sub i8 %cell311, 1
  store i8 %cell.dec312, ptr %cell.ptr310, align 1
  %idx313 = load i32, ptr %idx, align 4
  %cell.ptr314 = getelementptr inbounds i8, ptr %tape, i32 %idx313
  %cell315 = load i8, ptr %cell.ptr314, align 1
  %cell.dec316 = sub i8 %cell315, 1
  store i8 %cell.dec316, ptr %cell.ptr314, align 1
  %idx317 = load i32, ptr %idx, align 4
  %cell.ptr318 = getelementptr inbounds i8, ptr %tape, i32 %idx317
  %cell319 = load i8, ptr %cell.ptr318, align 1
  %cell.dec320 = sub i8 %cell319, 1
  store i8 %cell.dec320, ptr %cell.ptr318, align 1
  %idx321 = load i32, ptr %idx, align 4
  %cell.ptr322 = getelementptr inbounds i8, ptr %tape, i32 %idx321
  %cell323 = load i8, ptr %cell.ptr322, align 1
  %cell.dec324 = sub i8 %cell323, 1
  store i8 %cell.dec324, ptr %cell.ptr322, align 1
  %idx325 = load i32, ptr %idx, align 4
  %cell.ptr326 = getelementptr inbounds i8, ptr %tape, i32 %idx325
  %cell327 = load i8, ptr %cell.ptr326, align 1
  %cell.dec328 = sub i8 %cell327, 1
  store i8 %cell.dec328, ptr %cell.ptr326, align 1
  %idx329 = load i32, ptr %idx, align 4
  %cell.ptr330 = getelementptr inbounds i8, ptr %tape, i32 %idx329
  %cell331 = load i8, ptr %cell.ptr330, align 1
  %cell.zext332 = zext i8 %cell331 to i32
  %10 = call i32 @putchar(i32 %cell.zext332)
  %idx333 = load i32, ptr %idx, align 4
  %idx.inc334 = add i32 %idx333, 1
  store i32 %idx.inc334, ptr %idx, align 4
  %idx335 = load i32, ptr %idx, align 4
  %idx.inc336 = add i32 %idx335, 1
  store i32 %idx.inc336, ptr %idx, align 4
  %idx337 = load i32, ptr %idx, align 4
  %cell.ptr338 = getelementptr inbounds i8, ptr %tape, i32 %idx337
  %cell339 = load i8, ptr %cell.ptr338, align 1
  %cell.inc340 = add i8 %cell339, 1
  store i8 %cell.inc340, ptr %cell.ptr338, align 1
  %idx341 = load i32, ptr %idx, align 4
  %cell.ptr342 = getelementptr inbounds i8, ptr %tape, i32 %idx341
  %cell343 = load i8, ptr %cell.ptr342, align 1
  %cell.zext344 = zext i8 %cell343 to i32
  %11 = call i32 @putchar(i32 %cell.zext344)
  %idx345 = load i32, ptr %idx, align 4
  %idx.inc346 = add i32 %idx345, 1
  store i32 %idx.inc346, ptr %idx, align 4
  %idx347 = load i32, ptr %idx, align 4
  %cell.ptr348 = getelementptr inbounds i8, ptr %tape, i32 %idx347
  %cell349 = load i8, ptr %cell.ptr348, align 1
  %cell.inc350 = add i8 %cell349, 1
  store i8 %cell.inc350, ptr %cell.ptr348, align 1
  %idx351 = load i32, ptr %idx, align 4
  %cell.ptr352 = getelementptr inbounds i8, ptr %tape, i32 %idx351
  %cell353 = load i8, ptr %cell.ptr352, align 1
  %cell.inc354 = add i8 %cell353, 1
  store i8 %cell.inc354, ptr %cell.ptr352, align 1
  %idx355 = load i32, ptr %idx, align 4
  %cell.ptr356 = getelementptr inbounds i8, ptr %tape, i32 %idx355
  %cell357 = load i8, ptr %cell.ptr356, align 1
  %cell.zext358 = zext i8 %cell357 to i32
  %12 = call i32 @putchar(i32 %cell.zext358)
  ret i32 0

loop.cond50:                                      ; preds = %loop.body51, %loop.body
  %idx53 = load i32, ptr %idx, align 4
  %cell.ptr54 = getelementptr inbounds i8, ptr %tape, i32 %idx53
  %cell55 = load i8, ptr %cell.ptr54, align 1
  %cell.nz56 = icmp ne i8 %cell55, 0
  br i1 %cell.nz56, label %loop.body51, label %loop.end52

loop.body51:                                      ; preds = %loop.cond50
  %idx57 = load i32, ptr %idx, align 4
  %idx.inc58 = add i32 %idx57, 1
  store i32 %idx.inc58, ptr %idx, align 4
  %idx59 = load i32, ptr %idx, align 4
  %cell.ptr60 = getelementptr inbounds i8, ptr %tape, i32 %idx59
  %cell61 = load i8, ptr %cell.ptr60, align 1
  %cell.inc62 = add i8 %cell61, 1
  store i8 %cell.inc62, ptr %cell.ptr60, align 1
  %idx63 = load i32, ptr %idx, align 4
  %cell.ptr64 = getelementptr inbounds i8, ptr %tape, i32 %idx63
  %cell65 = load i8, ptr %cell.ptr64, align 1
  %cell.inc66 = add i8 %cell65, 1
  store i8 %cell.inc66, ptr %cell.ptr64, align 1
  %idx67 = load i32, ptr %idx, align 4
  %idx.inc68 = add i32 %idx67, 1
  store i32 %idx.inc68, ptr %idx, align 4
  %idx69 = load i32, ptr %idx, align 4
  %cell.ptr70 = getelementptr inbounds i8, ptr %tape, i32 %idx69
  %cell71 = load i8, ptr %cell.ptr70, align 1
  %cell.inc72 = add i8 %cell71, 1
  store i8 %cell.inc72, ptr %cell.ptr70, align 1
  %idx73 = load i32, ptr %idx, align 4
  %cell.ptr74 = getelementptr inbounds i8, ptr %tape, i32 %idx73
  %cell75 = load i8, ptr %cell.ptr74, align 1
  %cell.inc76 = add i8 %cell75, 1
  store i8 %cell.inc76, ptr %cell.ptr74, align 1
  %idx77 = load i32, ptr %idx, align 4
  %cell.ptr78 = getelementptr inbounds i8, ptr %tape, i32 %idx77
  %cell79 = load i8, ptr %cell.ptr78, align 1
  %cell.inc80 = add i8 %cell79, 1
  store i8 %cell.inc80, ptr %cell.ptr78, align 1
  %idx81 = load i32, ptr %idx, align 4
  %idx.inc82 = add i32 %idx81, 1
  store i32 %idx.inc82, ptr %idx, align 4
  %idx83 = load i32, ptr %idx, align 4
  %cell.ptr84 = getelementptr inbounds i8, ptr %tape, i32 %idx83
  %cell85 = load i8, ptr %cell.ptr84, align 1
  %cell.inc86 = add i8 %cell85, 1
  store i8 %cell.inc86, ptr %cell.ptr84, align 1
  %idx87 = load i32, ptr %idx, align 4
  %cell.ptr88 = getelementptr inbounds i8, ptr %tape, i32 %idx87
  %cell89 = load i8, ptr %cell.ptr88, align 1
  %cell.inc90 = add i8 %cell89, 1
  store i8 %cell.inc90, ptr %cell.ptr88, align 1
  %idx91 = load i32, ptr %idx, align 4
  %cell.ptr92 = getelementptr inbounds i8, ptr %tape, i32 %idx91
  %cell93 = load i8, ptr %cell.ptr92, align 1
  %cell.inc94 = add i8 %cell93, 1
  store i8 %cell.inc94, ptr %cell.ptr92, align 1
  %idx95 = load i32, ptr %idx, align 4
  %idx.inc96 = add i32 %idx95, 1
  store i32 %idx.inc96, ptr %idx, align 4
  %idx97 = load i32, ptr %idx, align 4
  %cell.ptr98 = getelementptr inbounds i8, ptr %tape, i32 %idx97
  %cell99 = load i8, ptr %cell.ptr98, align 1
  %cell.inc100 = add i8 %cell99, 1
  store i8 %cell.inc100, ptr %cell.ptr98, align 1
  %idx101 = load i32, ptr %idx, align 4
  %idx.dec = sub i32 %idx101, 1
  store i32 %idx.dec, ptr %idx, align 4
  %idx102 = load i32, ptr %idx, align 4
  %idx.dec103 = sub i32 %idx102, 1
  store i32 %idx.dec103, ptr %idx, align 4
  %idx104 = load i32, ptr %idx, align 4
  %idx.dec105 = sub i32 %idx104, 1
  store i32 %idx.dec105, ptr %idx, align 4
  %idx106 = load i32, ptr %idx, align 4
  %idx.dec107 = sub i32 %idx106, 1
  store i32 %idx.dec107, ptr %idx, align 4
  %idx108 = load i32, ptr %idx, align 4
  %cell.ptr109 = getelementptr inbounds i8, ptr %tape, i32 %idx108
  %cell110 = load i8, ptr %cell.ptr109, align 1
  %cell.dec = sub i8 %cell110, 1
  store i8 %cell.dec, ptr %cell.ptr109, align 1
  br label %loop.cond50

loop.end52:                                       ; preds = %loop.cond50
  %idx111 = load i32, ptr %idx, align 4
  %idx.inc112 = add i32 %idx111, 1
  store i32 %idx.inc112, ptr %idx, align 4
  %idx113 = load i32, ptr %idx, align 4
  %cell.ptr114 = getelementptr inbounds i8, ptr %tape, i32 %idx113
  %cell115 = load i8, ptr %cell.ptr114, align 1
  %cell.inc116 = add i8 %cell115, 1
  store i8 %cell.inc116, ptr %cell.ptr114, align 1
  %idx117 = load i32, ptr %idx, align 4
  %idx.inc118 = add i32 %idx117, 1
  store i32 %idx.inc118, ptr %idx, align 4
  %idx119 = load i32, ptr %idx, align 4
  %cell.ptr120 = getelementptr inbounds i8, ptr %tape, i32 %idx119
  %cell121 = load i8, ptr %cell.ptr120, align 1
  %cell.inc122 = add i8 %cell121, 1
  store i8 %cell.inc122, ptr %cell.ptr120, align 1
  %idx123 = load i32, ptr %idx, align 4
  %idx.inc124 = add i32 %idx123, 1
  store i32 %idx.inc124, ptr %idx, align 4
  %idx125 = load i32, ptr %idx, align 4
  %cell.ptr126 = getelementptr inbounds i8, ptr %tape, i32 %idx125
  %cell127 = load i8, ptr %cell.ptr126, align 1
  %cell.dec128 = sub i8 %cell127, 1
  store i8 %cell.dec128, ptr %cell.ptr126, align 1
  %idx129 = load i32, ptr %idx, align 4
  %idx.inc130 = add i32 %idx129, 1
  store i32 %idx.inc130, ptr %idx, align 4
  %idx131 = load i32, ptr %idx, align 4
  %idx.inc132 = add i32 %idx131, 1
  store i32 %idx.inc132, ptr %idx, align 4
  %idx133 = load i32, ptr %idx, align 4
  %cell.ptr134 = getelementptr inbounds i8, ptr %tape, i32 %idx133
  %cell135 = load i8, ptr %cell.ptr134, align 1
  %cell.inc136 = add i8 %cell135, 1
  store i8 %cell.inc136, ptr %cell.ptr134, align 1
  br label %loop.cond137

loop.cond137:                                     ; preds = %loop.body138, %loop.end52
  %idx140 = load i32, ptr %idx, align 4
  %cell.ptr141 = getelementptr inbounds i8, ptr %tape, i32 %idx140
  %cell142 = load i8, ptr %cell.ptr141, align 1
  %cell.nz143 = icmp ne i8 %cell142, 0
  br i1 %cell.nz143, label %loop.body138, label %loop.end139

loop.body138:                                     ; preds = %loop.cond137
  %idx144 = load i32, ptr %idx, align 4
  %idx.dec145 = sub i32 %idx144, 1
  store i32 %idx.dec145, ptr %idx, align 4
  br label %loop.cond137

loop.end139:                                      ; preds = %loop.cond137
  %idx146 = load i32, ptr %idx, align 4
  %idx.dec147 = sub i32 %idx146, 1
  store i32 %idx.dec147, ptr %idx, align 4
  %idx148 = load i32, ptr %idx, align 4
  %cell.ptr149 = getelementptr inbounds i8, ptr %tape, i32 %idx148
  %cell150 = load i8, ptr %cell.ptr149, align 1
  %cell.dec151 = sub i8 %cell150, 1
  store i8 %cell.dec151, ptr %cell.ptr149, align 1
  br label %loop.cond
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr writeonly captures(none), i8, i64, i1 immarg) #0

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: write) }
