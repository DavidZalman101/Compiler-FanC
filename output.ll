@.str0 = constant [23 x i8] c"Error division by zero\00"

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
define void @main() {
	; init_local_vars:
	%stack_base_size = add i32 3, 0
	%ptr_stack_base = alloca i32, i32 %stack_base_size
	store i32 0, i32* %ptr_stack_base

	; init pointers to local arguments
	%t0_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 0
	%t1_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 1
	%t2_stack_ptr = getelementptr i32,i32* %ptr_stack_base, i32 2

	%t0 = load i32, i32* %t0_stack_ptr
	%t1 = add i32 0, 5
	store i32 %t1, i32* %t0_stack_ptr

	%t2 = load i32, i32* %t1_stack_ptr
	%t3 = add i32 0, 5
	store i32 %t3, i32* %t1_stack_ptr

	%t4 = load i32, i32* %t2_stack_ptr
	%t5 = add i32 0, 2
	%t6 = load i32, i32* %t0_stack_ptr
	%t7 = load i32, i32* %t1_stack_ptr
	%t8 = sub i32 %t6, %t7
	%t12_is_zero = icmp eq i32 %t8, 0
	br i1 %t12_is_zero, label %label_2_zero_block, label %label_3_non_zero_block

label_2_zero_block:
	; check if dividing by zero
	%t13 = getelementptr [23 x i8], [23 x i8]* @.str0, i32 0, i32 0
	call void @print(i8* %t13)
	call void @exit(i32 0)
	unreachable

label_3_non_zero_block:
	; divide
	%t11 = sdiv i32 %t5, %t8
	store i32 %t11, i32* %t2_stack_ptr

	ret void
}

