
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
	%stack_base_size = add i32 1, 0
	%ptr_stack_base = alloca i32, i32 %stack_base_size
	store i32 0, i32* %ptr_stack_base

	; init pointers to local arguments
	%t0_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 0

	%t0 = load i32, i32* %t0_stack_ptr
	%t1 = add i32 0, 1
	%t2 = add i32 0, 1
	%t3 = add i32 0, 1
	%t4 = add i32 %t2, %t3
	%t5 = add i32 %t1, %t4
	store i32 %t5, i32* %t0_stack_ptr

	call i32 @foo(i32 1, i32 1);

	ret i32 0
}

define i32 @foo(i32, i32) {
	; init_local_vars:
	%stack_base_size = add i32 9, 0
	%ptr_stack_base = alloca i32, i32 %stack_base_size
	store i32 0, i32* %ptr_stack_base

	; init pointers to local arguments
	%t0_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 0
	%t1_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 1
	%t2_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 2
	%t3_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 3
	%t4_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 4
	%t5_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 5
	%t6_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 6
	%t7_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 7
	%t8_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 8

	%t6 = load i32, i32* %t0_stack_ptr
	%t7 = add i32 %0, %1
	store i32 %t7, i32* %t0_stack_ptr

	%t8 = sub i32 %0, %1
	store i32 %t8, i32* %t0_stack_ptr

	%t9 = load i32, i32* %t1_stack_ptr
	%t10 = mul i32 %1, %0
	store i32 %t10, i32* %t1_stack_ptr

	%t11 = load i32, i32* %t2_stack_ptr
	%t12 = load i32, i32* %t0_stack_ptr
	%t13 = sdiv i32 %t12, %1
	store i32 %t13, i32* %t2_stack_ptr

	%t14 = load i32, i32* %t3_stack_ptr
	%t15 = add i32 0, 100
	store i32 %t15, i32* %t3_stack_ptr

	%t16 = load i32, i32* %t4_stack_ptr
	%t17 = add i32 0, 200
	store i32 %t17, i32* %t4_stack_ptr

	%t18 = load i32, i32* %t5_stack_ptr
	%t19 = load i32, i32* %t3_stack_ptr
	%t20 = load i32, i32* %t4_stack_ptr
	%t21 = mul i32 %t19, %t20
	%t22 = and i32 %t21, 255
	store i32 %t22, i32* %t5_stack_ptr

	%t23 = load i32, i32* %t6_stack_ptr
	%t24 = add i32 0, 1
	store i32 %t24, i32* %t6_stack_ptr

	%t25 = load i32, i32* %t7_stack_ptr
	%t26 = add i32 0, 0
	store i32 %t26, i32* %t7_stack_ptr

	%t27 = load i32, i32* %t8_stack_ptr
	%t28 = load i32, i32* %t6_stack_ptr
	%t29 = load i32, i32* %t7_stack_ptr
	%t30 = xor i32 %t28, %t29
	store i32 %t30, i32* %t8_stack_ptr

	call void @printi(i32 %t30)


	ret i32 0
}

