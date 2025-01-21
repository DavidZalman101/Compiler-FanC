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
	%t1 = add i32 0, 0
	store i32 %t1, i32* %t0_stack_ptr

	%t2 = load i32, i32* %t1_stack_ptr
	%t3 = add i32 0, 1
	store i32 %t3, i32* %t1_stack_ptr

	%t4 = load i32, i32* %t2_stack_ptr
	%t6 = alloca i32, i32 1
	store i32 0, i32* %t6
	%t10 = alloca i32, i32 1
	store i32 0, i32* %t10
	%t12 = load i32, i32* %t0_stack_ptr
	%t11 = icmp eq i32 %t12, 0
	br i1 %t11, label %label_4, label %label_3
label_3:
	%t13 = load i32, i32* %t1_stack_ptr
	store i32 %t13, i32* %t10
	 br label %label_4
label_4:
	%t9 = load i32, i32* %t10
	%t7 = icmp eq i32 %t9, 1
	br i1 %t7, label %label_0, label %label_1
label_0:
	store i32 1, i32* %t6
	br label %label_2
label_1:
	%t15 = alloca i32, i32 1
	store i32 0, i32* %t15
	%t18 = load i32, i32* %t0_stack_ptr
	%t16 = icmp eq i32 %t18, 1
	br i1 %t16, label %label_5, label %label_6
label_5:
	store i32 1, i32* %t15
	br label %label_7
label_6:
	%t19 = load i32, i32* %t1_stack_ptr
	store i32 %t19, i32* %t15
	br label %label_7
label_7:
	%t14 = load i32, i32* %t15
	store i32 %t14, i32* %t6
	br label %label_2
label_2:
	%t5 = load i32, i32* %t6
	store i32 %t5, i32* %t2_stack_ptr

	ret void

}

