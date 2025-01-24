#include <stdio.h>


int foo() {
	printf("foo\n");
	return 0;
}

int bar() {
	printf("bar\n");
	return 1;
}

int baz(int arg0, int arg1) {
	return 2;
}

void main() {

	baz(foo(), bar());

	return;
}
