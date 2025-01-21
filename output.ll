
; print functions
declare i32 @scanf(i8*, ...)
declare i32 @printf(i8*, ...)
declare void @exit(i32)
@.int_specifier_scan = constant [3 x i8] c"%d\00"
@.int_specifier = constant [4 x i8] c"%d\0A\00"
@.str_specifier = constant [4 x i8] c"%s\0A\00"

define i32 @readi(i32) {
    %ret_val = alloca i32
    %spec_ptr = getelementptr [3 x i8], [3 x i8]* @.int_specifier_scan, i32 0, i32 0
    call i32 (i8*, ...) @scanf(i8* %spec_ptr, i32* %ret_val)
    %val = load i32, i32* %ret_val
    ret i32 %val
}

define void @printi(i32) {
    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0
    call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)
    ret void
}

define void @print(i8*) {
    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0
    call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)
    ret void
}

;functions
define i32 @main() {
	; init_local_vars:
	%stack_base_size = add i32 4, 0
	%ptr_stack_base = alloca i32, i32 %stack_base_size
	store i32 0, i32* %ptr_stack_base

	; init pointers to local arguments
	%t0_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 0
	%t1_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 1
	%t2_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 2
	%t3_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 3

	%t0 = load i32, i32* %t0_stack_ptr
	store i32 0, i32* %t0_stack_ptr

	%t1 = load i32, i32* %t1_stack_ptr
	store i32 0, i32* %t1_stack_ptr

	%t2 = load i32, i32* %t2_stack_ptr
	store i32 0, i32* %t2_stack_ptr


	%t3 = load i32, i32* %t1_stack_ptr
	store i32 0, i32* %t1_stack_ptr

	%t4 = load i32, i32* %t2_stack_ptr
	store i32 0, i32* %t2_stack_ptr

	%t5 = load i32, i32* %t3_stack_ptr
	%t6 = load i32, i32* %t1_stack_ptr
	%t7 = load i32, i32* %t2_stack_ptr
	%t8 = add i32 %t6, %t7
	store i32 %t8, i32* %t3_stack_ptr


	ret i32 0
}

define i32 @foo(i32, i32) {
	; init_local_vars:
	%stack_base_size = add i32 5, 0
	%ptr_stack_base = alloca i32, i32 %stack_base_size
	store i32 0, i32* %ptr_stack_base

	; init pointers to local arguments
	%t0_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 0
	%t1_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 1
	%t2_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 2
	%t3_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 3
	%t4_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 4

	%t9 = load i32, i32* %t0_stack_ptr
	%t10 = add i32 0, 0
	store i32 %t10, i32* %t0_stack_ptr

	%t11 = load i32, i32* %t1_stack_ptr
	%t12 = add i32 0, 1
	store i32 %t12, i32* %t1_stack_ptr

	%t13 = load i32, i32* %t2_stack_ptr
	%t14 = add i32 0, 2
	store i32 %t14, i32* %t2_stack_ptr

	%t15 = load i32, i32* %t3_stack_ptr
	%t16 = add i32 0, 3
	store i32 %t16, i32* %t3_stack_ptr

	%t17 = load i32, i32* %t4_stack_ptr
	%t18 = load i32, i32* %t2_stack_ptr
	%t19 = load i32, i32* %t3_stack_ptr
	%t20 = add i32 %t18, %t19
	%t21 = load i32, i32* %t0_stack_ptr
	%t22 = add i32 %t20, %t21
	store i32 %t22, i32* %t4_stack_ptr


	%t23 = load i32, i32* %t2_stack_ptr
	%t24 = load i32, i32* %t0_stack_ptr
	%t25 = load i32, i32* %t1_stack_ptr
	%t26 = add i32 %t24, %t25
	store i32 %t26, i32* %t2_stack_ptr


	ret i32 0
}

